#pragma once
#include <cstdint>
#include <map>
#include <enet/enet.h>
#include "entity.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_STATE,
  E_SERVER_TO_CLIENT_SNAPSHOT,
  E_SERVER_TO_CLIENT_SCORES
};

void send_join(ENetPeer *peer);
void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_scores(ENetPeer *peer, std::map<uint16_t, size_t> &scoreMap);
void send_set_controlled_entity(ENetPeer *peer, uint16_t eid);
// void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y);
// void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y);
void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y, size_t size);
void send_snapshot(ENetPeer *peer, const Entity &ent);

MessageType get_packet_type(ENetPacket *packet);

void deserialize_new_entity(ENetPacket *packet, Entity &ent);
void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid);
// void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y);
// void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y);
void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y, size_t &size);
void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, size_t &size);
