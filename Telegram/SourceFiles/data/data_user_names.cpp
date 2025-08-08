/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "data/data_user_names.h"

namespace Data {

UsernamesInfo::UsernamesInfo() = default;

void UsernamesInfo::setUsername(const QString &username) {
	if (_usernames.empty()) {
		if (username.isEmpty()) {
			_indexEditableUsername = -1;
		} else {
			_usernames.push_back(username);
			_indexEditableUsername = 0;
		}
	} else if ((_indexEditableUsername < 0)
			|| (_indexEditableUsername >= _usernames.size())) {
		if (username.isEmpty()) {
			_indexEditableUsername = -1;
		} else {
			_usernames.push_back(username);
			_indexEditableUsername = 0;
		}
	} else if (_usernames[_indexEditableUsername] != username) {
		if (username.isEmpty()) {
			_usernames.erase(begin(_usernames) + _indexEditableUsername);
			_indexEditableUsername = -1;
		} else {
			_usernames[_indexEditableUsername] = username;
		}
	}
}

void UsernamesInfo::setUsernames(const Usernames &usernames) {
  auto editableIndex = -1;
  auto newUsernames =
      ranges::views::all(usernames) |
      ranges::views::transform(
          [](const Data::Username &username) { return username.username; }) |
      ranges::to_vector;

  if (!ranges::equal(_usernames, newUsernames)) {
    _usernames = std::move(newUsernames);
  }
  for (auto i = 0; i < usernames.size(); ++i) {
    if (usernames[i].editable) {
      editableIndex = i;
      break;
    }
  }
  _indexEditableUsername = editableIndex;
}

QString UsernamesInfo::username() const {
	return _usernames.empty() ? QString() : _usernames.front();
}

QString UsernamesInfo::editableUsername() const {
	return (_indexEditableUsername < 0)
		? QString()
		: _usernames[_indexEditableUsername];
}

const std::vector<QString> &UsernamesInfo::usernames() const {
	return _usernames;
}

bool UsernamesInfo::isEditable(const QString &username) const {
	return (_indexEditableUsername >= 0)
		&& (_indexEditableUsername < _usernames.size())
		&& (_usernames[_indexEditableUsername] == username);
}

} // namespace Data
