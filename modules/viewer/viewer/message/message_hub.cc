#include "viewer/message/message_hub.h"

namespace crdc {
namespace airi {

MessageHub::MessageHub() {
  node_ = apollo::cyber::CreateNode("viewer", "crdc_airi");
  auto channel_manager =
      apollo::cyber::service_discovery::TopologyManager::Instance()->channel_manager();
  channel_manager->AddChangeListener(
      std::bind(&MessageHub::onChannelChange, this, std::placeholders::_1));
}

apollo::cyber::Node *MessageHub::node() { return node_.get(); }

void MessageHub::onChannelChange(const apollo::cyber::proto::ChangeMsg &msg) {
  if (msg.has_role_type() && msg.role_type() == apollo::cyber::RoleType::ROLE_WRITER &&
      msg.has_role_attr()) {
    const auto &role_attr = msg.role_attr();
    const auto &channel = role_attr.channel_name();
    const auto &type = role_attr.message_type();

    if (msg.operate_type() == apollo::cyber::proto::OPT_JOIN) {
      map_channel_type_[channel] = type;
      map_type_channels_[type].insert(channel);
      
      if (map_channel_subscriptions_.find(channel) != map_channel_subscriptions_.end() ||
          map_type_subscriptions_.find(type) != map_type_subscriptions_.end()) {
        addReader(channel);
      }
    } else {
      map_channel_type_.erase(channel);
      map_type_channels_[type].erase(channel);
      removeReader(channel);
    }
  }
}

void MessageHub::addReader(const std::string &channel) {
  if (map_channel_reader_.find(channel) == map_channel_reader_.end()) {
    map_channel_reader_[channel] = node_->CreateReader<RawMessage>(
      channel, std::bind(&MessageHub::callbackInner, this, channel, std::placeholders::_1)); 
  }
}

void MessageHub::removeReader(const std::string &channel) {
  auto it = map_channel_reader_.find(channel);
  if (it != map_channel_reader_.end()) {
    map_channel_reader_.erase(it);
  }
}

void MessageHub::callbackInner(const std::string &channel, const std::shared_ptr<RawMessage> &msg) {
  // callback channel subscriptions
  if (map_channel_subscriptions_.find(channel) != map_channel_subscriptions_.end()) {
    for (const auto &subscription : map_channel_subscriptions_[channel]) {
      subscription->onMessage(channel, msg->message.data(), msg->message.size());
    }
  }

  // callback type subscriptions
  if (map_channel_type_.find(channel) != map_channel_type_.end()) {
    const auto &type = map_channel_type_[channel];
    if (map_type_subscriptions_.find(type) != map_type_subscriptions_.end()) {
      for (const auto &subscription : map_type_subscriptions_[type]) {
        subscription->onMessage(channel, msg->message.data(), msg->message.size());
      }
    }
  }
}

}  // namespace airi
}  // namespace crdc
