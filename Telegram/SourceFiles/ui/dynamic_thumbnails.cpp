/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/dynamic_thumbnails.h"

#include "data/data_changes.h"
#include "data/data_document.h"
#include "data/data_document_media.h"
#include "data/data_file_origin.h"
#include "data/data_peer.h"
#include "data/data_photo.h"
#include "data/data_photo_media.h"
#include "data/data_session.h"
#include "data/stickers/data_custom_emoji.h"
#include "data/data_story.h"
#include "main/main_session.h"
#include "ui/empty_userpic.h"
#include "ui/dynamic_image.h"
#include "ui/painter.h"
#include "ui/userpic_view.h"

#include <QHash>
#include <optional>
#include <mutex>

namespace Ui {
namespace {

struct CacheKey {
        QString id;
        int size = 0;
        bool operator==(const CacheKey &other) const noexcept {
                return (size == other.size) && (id == other.id);
        }
};

inline uint qHash(const CacheKey &key, uint seed = 0) {
        return ::qHash(key.id, seed) ^ uint(key.size);
}

class ThumbnailCache {
public:
        static ThumbnailCache &instance() {
                static ThumbnailCache cache;
                return cache;
        }

        std::optional<QImage> get(const CacheKey &key) {
                std::lock_guard<std::mutex> lock(_mutex);
                const auto it = _cache.find(key);
                if (it != _cache.end()) {
                        return it.value();
                }
                return std::nullopt;
        }

        void put(const CacheKey &key, QImage image) {
                std::lock_guard<std::mutex> lock(_mutex);
                _cache.insert(key, std::move(image));
        }

private:
        std::mutex _mutex;
        QHash<CacheKey, QImage> _cache;
};

template <typename Generator, typename Ready>
void loadCachedAsync(const CacheKey &key, Generator generator, Ready ready) {
        if (const auto cached = ThumbnailCache::instance().get(key)) {
                ready(*cached);
                return;
        }
        crl::async([=]() mutable {
                auto image = generator();
                ThumbnailCache::instance().put(key, image);
                crl::on_main([image = std::move(image), ready]() mutable {
                        ready(image);
                });
        });
}

class PeerUserpic final : public DynamicImage {
public:
	PeerUserpic(not_null<PeerData*> peer, bool forceRound);

	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	struct Subscribed {
		explicit Subscribed(Fn<void()> callback)
		: callback(std::move(callback)) {
		}

		Ui::PeerUserpicView view;
		Fn<void()> callback;
		InMemoryKey key;
		int paletteVersion = 0;
		rpl::lifetime photoLifetime;
		rpl::lifetime downloadLifetime;
	};

	[[nodiscard]] bool waitingUserpicLoad() const;
	void processNewPhoto();

	const not_null<PeerData*> _peer;
	QImage _frame;
	std::unique_ptr<Subscribed> _subscribed;
	bool _forceRound = false;

};

class StoryThumbnail : public DynamicImage {
public:
	explicit StoryThumbnail(FullStoryId id);

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

protected:
	struct Thumb {
		Image *image = nullptr;
		bool blurred = false;
	};

	[[nodiscard]] FullStoryId id() const;

	[[nodiscard]] virtual Main::Session &session() = 0;
	[[nodiscard]] virtual Thumb loaded(FullStoryId id) = 0;
	virtual void clear() = 0;

private:
        const FullStoryId _id;
        QImage _full;
        rpl::lifetime _subscription;
        QImage _prepared;
        bool _blurred = false;
        Fn<void()> _callback;

};

class PhotoThumbnail final : public StoryThumbnail {
public:
	PhotoThumbnail(not_null<PhotoData*> photo, FullStoryId id);

	std::shared_ptr<DynamicImage> clone() override;

private:
	Main::Session &session() override;
	Thumb loaded(FullStoryId id) override;
	void clear() override;

	const not_null<PhotoData*> _photo;
	std::shared_ptr<Data::PhotoMedia> _media;

};

class VideoThumbnail final : public StoryThumbnail {
public:
	VideoThumbnail(not_null<DocumentData*> video, FullStoryId id);

	std::shared_ptr<DynamicImage> clone() override;

private:
	Main::Session &session() override;
	Thumb loaded(FullStoryId id) override;
	void clear() override;

	const not_null<DocumentData*> _video;
	std::shared_ptr<Data::DocumentMedia> _media;

};

class EmptyThumbnail final : public DynamicImage {
public:
	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	QImage _cached;

};

class SavedMessagesUserpic final : public DynamicImage {
public:
	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	QImage _frame;
	int _paletteVersion = 0;

};

class RepliesUserpic final : public DynamicImage {
public:
	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	QImage _frame;
	int _paletteVersion = 0;

};

class HiddenAuthorUserpic final : public DynamicImage {
public:
	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	QImage _frame;
	int _paletteVersion = 0;

};

class IconThumbnail final : public DynamicImage {
public:
	explicit IconThumbnail(const style::icon &icon);

	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	const style::icon &_icon;
	int _paletteVersion = 0;
	QImage _frame;

};

class EmojiThumbnail final : public DynamicImage {
public:
	EmojiThumbnail(not_null<Data::Session*> owner, const QString &data);

	std::shared_ptr<DynamicImage> clone() override;

	QImage image(int size) override;
	void subscribeToUpdates(Fn<void()> callback) override;

private:
	const not_null<Data::Session*> _owner;
	const QString _data;
	std::unique_ptr<Ui::Text::CustomEmoji> _emoji;
	QImage _frame;

};

PeerUserpic::PeerUserpic(not_null<PeerData*> peer, bool forceRound)
: _peer(peer)
, _forceRound(forceRound) {
}

std::shared_ptr<DynamicImage> PeerUserpic::clone() {
	return std::make_shared<PeerUserpic>(_peer, _forceRound);
}

QImage PeerUserpic::image(int size) {
	Expects(_subscribed != nullptr);

        const auto good = (_frame.width() == size * _frame.devicePixelRatio());
        const auto key = _peer->userpicUniqueKey(_subscribed->view);
        const auto paletteVersion = style::PaletteVersion();
        if (!good
                || (_subscribed->paletteVersion != paletteVersion
                        && _peer->useEmptyUserpic(_subscribed->view))
                || (_subscribed->key != key && !waitingUserpicLoad())) {
                _subscribed->key = key;
                _subscribed->paletteVersion = paletteVersion;

                const auto cacheKey = CacheKey{
                        QString("peer:%1:%2:%3:%4")
                                .arg(_peer->id.value)
                                .arg(size)
                                .arg(_forceRound ? 1 : 0)
                                .arg(paletteVersion),
                        size };
                const auto view = _subscribed->view;
                loadCachedAsync(cacheKey, [=]() mutable {
                        const auto ratio = style::DevicePixelRatio();
                        QImage frame(
                                QSize(size, size) * ratio,
                                QImage::Format_ARGB32_Premultiplied);
                        frame.setDevicePixelRatio(ratio);
                        frame.fill(Qt::transparent);

                        auto v = view;
                        auto p = Painter(&frame);
                        if (!_forceRound) {
                                _peer->paintUserpic(p, v, 0, 0, size);
                        } else if (const auto cloud = _peer->userpicCloudImage(v)) {
                                const auto full = size * style::DevicePixelRatio();
                                Ui::ValidateUserpicCache(v, cloud, nullptr, full, false);
                                p.drawImage(QRect(0, 0, size, size), v.cached);
                        } else {
                                const auto full = size * style::DevicePixelRatio();
                                const auto r = full / 2.;
                                const auto empty = _peer->generateUserpicImage(v, full, r);
                                p.drawImage(QRect(0, 0, size, size), empty);
                        }
                        return frame;
                }, [=](QImage img) {
                        _frame = img;
                        if (_subscribed) {
                                _subscribed->callback();
                        }
                });
        }
        return _frame;
}

bool PeerUserpic::waitingUserpicLoad() const {
	return _peer->hasUserpic() && _peer->useEmptyUserpic(_subscribed->view);
}

void PeerUserpic::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_subscribed = nullptr;
		return;
	}
	_subscribed = std::make_unique<Subscribed>(std::move(callback));

	_peer->session().changes().peerUpdates(
		_peer,
		Data::PeerUpdate::Flag::Photo
	) | rpl::start_with_next([=] {
		_subscribed->callback();
		processNewPhoto();
	}, _subscribed->photoLifetime);

	processNewPhoto();
}

void PeerUserpic::processNewPhoto() {
	Expects(_subscribed != nullptr);

	if (!waitingUserpicLoad()) {
		_subscribed->downloadLifetime.destroy();
		return;
	}
	_peer->session().downloaderTaskFinished(
	) | rpl::filter([=] {
		return !waitingUserpicLoad();
	}) | rpl::start_with_next([=] {
		_subscribed->callback();
		_subscribed->downloadLifetime.destroy();
	}, _subscribed->downloadLifetime);
}

StoryThumbnail::StoryThumbnail(FullStoryId id)
: _id(id) {
}

QImage StoryThumbnail::image(int size) {
        const auto ratio = style::DevicePixelRatio();
        if (_prepared.width() != size * ratio) {
                const auto cacheKey = CacheKey{
                        QString("story:%1:%2:%3")
                                .arg(_id.peer.value)
                                .arg(_id.story)
                                .arg(size),
                        size };
                const auto full = _full;
                loadCachedAsync(cacheKey, [=]() mutable {
                        QImage result;
                        if (full.isNull()) {
                                result = QImage(
                                        QSize(size, size) * ratio,
                                        QImage::Format_ARGB32_Premultiplied);
                                result.fill(Qt::black);
                        } else {
                                const auto width = full.width();
                                const auto skip = std::max((full.height() - width) / 2, 0);
                                result = full.copy(0, skip, width, width).scaled(
                                        QSize(size, size) * ratio,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation);
                        }
                        result = Images::Circle(std::move(result));
                        result.setDevicePixelRatio(ratio);
                        return result;
                }, [=](QImage img) {
                        _prepared = img;
                        if (_callback) {
                                _callback();
                        }
                });
        }
        return _prepared;
}

void StoryThumbnail::subscribeToUpdates(Fn<void()> callback) {
        _subscription.destroy();
        _callback = callback;
        if (!_callback) {
                clear();
                return;
        } else if (!_full.isNull() && !_blurred) {
                return;
        }
        const auto thumbnail = loaded(_id);
        if (const auto image = thumbnail.image) {
                _full = image->original();
        }
        _blurred = thumbnail.blurred;
        if (!_blurred) {
                _prepared = QImage();
        } else {
                _subscription = session().downloaderTaskFinished(
                ) | rpl::filter([=] {
                        const auto thumbnail = loaded(_id);
                        if (!thumbnail.blurred) {
                                _full = thumbnail.image->original();
                                _prepared = QImage();
                                _blurred = false;
                                return true;
                        }
                        return false;
                }) | rpl::take(1) | rpl::start_with_next([=] {
                        if (_callback) {
                                _callback();
                        }
                });
        }
}

FullStoryId StoryThumbnail::id() const {
	return _id;
}

PhotoThumbnail::PhotoThumbnail(not_null<PhotoData*> photo, FullStoryId id)
: StoryThumbnail(id)
, _photo(photo) {
}

std::shared_ptr<DynamicImage> PhotoThumbnail::clone() {
	return std::make_shared<PhotoThumbnail>(_photo, id());
}

Main::Session &PhotoThumbnail::session() {
	return _photo->session();
}

StoryThumbnail::Thumb PhotoThumbnail::loaded(FullStoryId id) {
	if (!_media) {
		_media = _photo->createMediaView();
		_media->wanted(Data::PhotoSize::Small, id);
	}
	if (const auto small = _media->image(Data::PhotoSize::Small)) {
		return { .image = small };
	}
	return { .image = _media->thumbnailInline(), .blurred = true };
}

void PhotoThumbnail::clear() {
	_media = nullptr;
}

VideoThumbnail::VideoThumbnail(
	not_null<DocumentData*> video,
	FullStoryId id)
: StoryThumbnail(id)
, _video(video) {
}

std::shared_ptr<DynamicImage> VideoThumbnail::clone() {
	return std::make_shared<VideoThumbnail>(_video, id());
}

Main::Session &VideoThumbnail::session() {
	return _video->session();
}

StoryThumbnail::Thumb VideoThumbnail::loaded(FullStoryId id) {
	if (!_media) {
		_media = _video->createMediaView();
		_media->thumbnailWanted(id);
	}
	if (const auto small = _media->thumbnail()) {
		return { .image = small };
	}
	return { .image = _media->thumbnailInline(), .blurred = true };
}

void VideoThumbnail::clear() {
	_media = nullptr;
}

std::shared_ptr<DynamicImage> EmptyThumbnail::clone() {
	return std::make_shared<EmptyThumbnail>();
}

QImage EmptyThumbnail::image(int size) {
	const auto ratio = style::DevicePixelRatio();
	if (_cached.width() != size * ratio) {
		_cached = QImage(
			QSize(size, size) * ratio,
			QImage::Format_ARGB32_Premultiplied);
		_cached.fill(Qt::black);
		_cached.setDevicePixelRatio(ratio);
	}
	return _cached;
}

void EmptyThumbnail::subscribeToUpdates(Fn<void()> callback) {
}

std::shared_ptr<DynamicImage> SavedMessagesUserpic::clone() {
	return std::make_shared<SavedMessagesUserpic>();
}

QImage SavedMessagesUserpic::image(int size) {
	const auto good = (_frame.width() == size * _frame.devicePixelRatio());
	const auto paletteVersion = style::PaletteVersion();
	if (!good || _paletteVersion != paletteVersion) {
		_paletteVersion = paletteVersion;

		const auto ratio = style::DevicePixelRatio();
		if (!good) {
			_frame = QImage(
				QSize(size, size) * ratio,
				QImage::Format_ARGB32_Premultiplied);
			_frame.setDevicePixelRatio(ratio);
		}
		_frame.fill(Qt::transparent);

		auto p = Painter(&_frame);
		Ui::EmptyUserpic::PaintSavedMessages(p, 0, 0, size, size);
	}
	return _frame;
}

void SavedMessagesUserpic::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_frame = {};
	}
}

std::shared_ptr<DynamicImage> RepliesUserpic::clone() {
	return std::make_shared<RepliesUserpic>();
}

QImage RepliesUserpic::image(int size) {
	const auto good = (_frame.width() == size * _frame.devicePixelRatio());
	const auto paletteVersion = style::PaletteVersion();
	if (!good || _paletteVersion != paletteVersion) {
		_paletteVersion = paletteVersion;

		const auto ratio = style::DevicePixelRatio();
		if (!good) {
			_frame = QImage(
				QSize(size, size) * ratio,
				QImage::Format_ARGB32_Premultiplied);
			_frame.setDevicePixelRatio(ratio);
		}
		_frame.fill(Qt::transparent);

		auto p = Painter(&_frame);
		Ui::EmptyUserpic::PaintRepliesMessages(p, 0, 0, size, size);
	}
	return _frame;
}

void RepliesUserpic::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_frame = {};
	}
}

std::shared_ptr<DynamicImage> HiddenAuthorUserpic::clone() {
	return std::make_shared<HiddenAuthorUserpic>();
}

QImage HiddenAuthorUserpic::image(int size) {
	const auto good = (_frame.width() == size * _frame.devicePixelRatio());
	const auto paletteVersion = style::PaletteVersion();
	if (!good || _paletteVersion != paletteVersion) {
		_paletteVersion = paletteVersion;

		const auto ratio = style::DevicePixelRatio();
		if (!good) {
			_frame = QImage(
				QSize(size, size) * ratio,
				QImage::Format_ARGB32_Premultiplied);
			_frame.setDevicePixelRatio(ratio);
		}
		_frame.fill(Qt::transparent);

		auto p = Painter(&_frame);
		Ui::EmptyUserpic::PaintHiddenAuthor(p, 0, 0, size, size);
	}
	return _frame;
}

void HiddenAuthorUserpic::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_frame = {};
	}
}

IconThumbnail::IconThumbnail(const style::icon &icon) : _icon(icon) {
}

std::shared_ptr<DynamicImage> IconThumbnail::clone() {
	return std::make_shared<IconThumbnail>(_icon);
}

QImage IconThumbnail::image(int size) {
	const auto good = (_frame.width() == size * _frame.devicePixelRatio());
	const auto paletteVersion = style::PaletteVersion();
	if (!good || _paletteVersion != paletteVersion) {
		_paletteVersion = paletteVersion;

		const auto ratio = style::DevicePixelRatio();
		if (!good) {
			_frame = QImage(
				QSize(size, size) * ratio,
				QImage::Format_ARGB32_Premultiplied);
			_frame.setDevicePixelRatio(ratio);
		}
		_frame.fill(Qt::transparent);

		auto p = Painter(&_frame);
		_icon.paintInCenter(p, QRect(0, 0, size, size));
	}
	return _frame;
}

void IconThumbnail::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_frame = {};
	}
}

EmojiThumbnail::EmojiThumbnail(
	not_null<Data::Session*> owner,
	const QString &data)
: _owner(owner)
, _data(data) {
}

void EmojiThumbnail::subscribeToUpdates(Fn<void()> callback) {
	if (!callback) {
		_emoji = nullptr;
		return;
	}
	_emoji = _owner->customEmojiManager().create(
		_data,
		std::move(callback),
		Data::CustomEmojiSizeTag::Large);
}

std::shared_ptr<DynamicImage> EmojiThumbnail::clone() {
	return std::make_shared<EmojiThumbnail>(_owner, _data);
}

QImage EmojiThumbnail::image(int size) {
	Expects(_emoji != nullptr);

	const auto ratio = style::DevicePixelRatio();
	const auto good = (_frame.width() == size * _frame.devicePixelRatio());
	if (!good) {
		_frame = QImage(
			QSize(size, size) * ratio,
			QImage::Format_ARGB32_Premultiplied);
		_frame.setDevicePixelRatio(ratio);
	}
	_frame.fill(Qt::transparent);

	auto p = Painter(&_frame);
	_emoji->paint(p, {
		.textColor = st::windowBoldFg->c,
		.now = crl::now(),
		.position = QPoint(0, 0),
		.paused = false,
	});
	p.end();

	return _frame;
}

} // namespace

std::shared_ptr<DynamicImage> MakeUserpicThumbnail(
		not_null<PeerData*> peer,
		bool forceRound) {
	return std::make_shared<PeerUserpic>(peer, forceRound);
}

std::shared_ptr<DynamicImage> MakeSavedMessagesThumbnail() {
	return std::make_shared<SavedMessagesUserpic>();
}

std::shared_ptr<DynamicImage> MakeRepliesThumbnail() {
	return std::make_shared<RepliesUserpic>();
}

std::shared_ptr<DynamicImage> MakeHiddenAuthorThumbnail() {
	return std::make_shared<HiddenAuthorUserpic>();
}

std::shared_ptr<DynamicImage> MakeStoryThumbnail(
		not_null<Data::Story*> story) {
	using Result = std::shared_ptr<DynamicImage>;
	const auto id = story->fullId();
	return v::match(story->media().data, [](v::null_t) -> Result {
		return std::make_shared<EmptyThumbnail>();
	}, [&](not_null<PhotoData*> photo) -> Result {
		return std::make_shared<PhotoThumbnail>(photo, id);
	}, [&](not_null<DocumentData*> video) -> Result {
		return std::make_shared<VideoThumbnail>(video, id);
	});
}

std::shared_ptr<DynamicImage> MakeIconThumbnail(const style::icon &icon) {
	return std::make_shared<IconThumbnail>(icon);
}

std::shared_ptr<DynamicImage> MakeEmojiThumbnail(
		not_null<Data::Session*> owner,
		const QString &data) {
	return std::make_shared<EmojiThumbnail>(owner, data);
}

} // namespace Ui
