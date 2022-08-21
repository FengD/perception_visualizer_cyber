#pragma once

#include <cyber/cyber.h>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include "viewer/message/subscription.h"

namespace crdc {
namespace airi {

using apollo::cyber::message::RawMessage;

class MessageHub {
 public:
  MessageHub();

 public:
  apollo::cyber::Node *node();

  template <typename MessageT>
  void subscribe(
      const std::string &channel,
      const std::function<void(const std::string &, const std::shared_ptr<MessageT> &)> &callback) {
    // add subscription
    auto subscription = std::make_shared<Subscription<MessageT>>(callback);
    map_channel_subscriptions_[channel].push_back(subscription);

    // add reader if needed
    addReader(channel);
  }

  template <typename MessageT>
  void subscribe(
      const std::function<void(const std::string &, const std::shared_ptr<MessageT> &)> &callback) {
    // get type name
    const auto &type = MessageT().GetTypeName();

    // add subscription
    auto subscription = std::make_shared<Subscription<MessageT>>(callback);
    map_type_subscriptions_[type].push_back(subscription);

    // update readers
    for (const auto &channel : map_type_channels_[type]) {
      addReader(channel);
    }
  }

 protected:
  void onChannelChange(const apollo::cyber::proto::ChangeMsg &msg);

  void addReader(const std::string &channel);

  void removeReader(const std::string &channel);

  void callbackInner(const std::string &channel, const std::shared_ptr<RawMessage> &msg);

 protected:
  std::unique_ptr<apollo::cyber::Node> node_;

  std::unordered_map<std::string, std::string> map_channel_type_;
  std::unordered_map<std::string, std::set<std::string>> map_type_channels_;
  std::unordered_map<std::string, std::list<std::shared_ptr<SubscriptionBase>>>
      map_channel_subscriptions_;
  std::unordered_map<std::string, std::list<std::shared_ptr<SubscriptionBase>>>
      map_type_subscriptions_;
  std::unordered_map<std::string, std::shared_ptr<apollo::cyber::ReaderBase>> map_channel_reader_;
};

}  // namespace airi
}  // namespace crdc
