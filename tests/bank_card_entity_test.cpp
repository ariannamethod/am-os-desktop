#include <QtCore/QString>
#include <QtCore/QVector>
#include <cassert>
#include <variant>

// Minimal stubs to mimic Telegram types.
struct MTP_int {
  int v;
  explicit MTP_int(int value) : v(value) {}
};

struct MTPDmessageEntityBankCard {
  MTP_int voffset_;
  MTP_int vlength_;
  MTPDmessageEntityBankCard(MTP_int offset, MTP_int length)
      : voffset_(offset), vlength_(length) {}
  MTP_int voffset() const { return voffset_; }
  MTP_int vlength() const { return vlength_; }
};

using MTPMessageEntity = std::variant<MTPDmessageEntityBankCard>;

enum class EntityType {
  BankCard,
};

struct EntityInText {
  EntityType type;
  int offset;
  int length;
  QString data;
};

using EntitiesInText = QVector<EntityInText>;

EntitiesInText EntitiesFromMTP(const QVector<MTPMessageEntity> &entities) {
  EntitiesInText result;
  for (const auto &entity : entities) {
    std::visit(
        [&](const auto &d) {
          result.push_back(
              {EntityType::BankCard, d.voffset().v, d.vlength().v, {}});
        },
        entity);
  }
  return result;
}

QVector<MTPMessageEntity> EntitiesToMTP(const EntitiesInText &entities) {
  QVector<MTPMessageEntity> result;
  for (const auto &entity : entities) {
    result.push_back(MTPDmessageEntityBankCard(MTP_int(entity.offset),
                                               MTP_int(entity.length)));
  }
  return result;
}

int main() {
  EntitiesInText original;
  original.push_back({EntityType::BankCard, 3, 4, {}});
  auto mtp = EntitiesToMTP(original);
  auto round = EntitiesFromMTP(mtp);
  assert(round.size() == 1);
  assert(round[0].type == EntityType::BankCard);
  assert(round[0].offset == 3);
  assert(round[0].length == 4);
  return 0;
}
