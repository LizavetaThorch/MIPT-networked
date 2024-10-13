#include "protocol.h"
#include <cstring> // memcpy

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  *packet->data = E_CLIENT_TO_SERVER_JOIN;

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity),
                                          ENET_PACKET_FLAG_RELIABLE);
  uint8_t *ptr = packet->data;
  *ptr = E_SERVER_TO_CLIENT_NEW_ENTITY;
  ptr += sizeof(uint8_t);
  memcpy(ptr, &ent, sizeof(Entity));
  ptr += sizeof(Entity);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                          ENET_PACKET_FLAG_RELIABLE);
  uint8_t *ptr = packet->data;
  *ptr = E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY;
  ptr += sizeof(uint8_t);
  memcpy(ptr, &eid, sizeof(uint16_t));
  ptr += sizeof(uint16_t);

  enet_peer_send(peer, 0, packet);
}

void send_entity_input(ENetPeer *peer, uint16_t eid, float thr, float steer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) + 2 * sizeof(float),
                                          ENET_PACKET_FLAG_UNSEQUENCED);
  uint8_t *ptr = packet->data;
  *ptr = E_CLIENT_TO_SERVER_INPUT;
  ptr += sizeof(uint8_t);
  memcpy(ptr, &eid, sizeof(uint16_t));
  ptr += sizeof(uint16_t);
  memcpy(ptr, &thr, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &steer, sizeof(float));
  ptr += sizeof(float);

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float ori, enet_uint32 timeStamp)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) + 3 * sizeof(float) + sizeof(enet_uint32),
                                          ENET_PACKET_FLAG_UNSEQUENCED);
  uint8_t *ptr = packet->data;
  *ptr = E_SERVER_TO_CLIENT_SNAPSHOT;
  ptr += sizeof(uint8_t);
  memcpy(ptr, &eid, sizeof(uint16_t));
  ptr += sizeof(uint16_t);
  memcpy(ptr, &x, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &y, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &ori, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &timeStamp, sizeof(enet_uint32));

  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  uint8_t *ptr = packet->data;
  ptr += sizeof(uint8_t);
  ent = *(Entity *)(ptr);
  ptr += sizeof(Entity);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  uint8_t *ptr = packet->data;
  ptr += sizeof(uint8_t);
  eid = *(uint16_t *)(ptr);
  ptr += sizeof(uint16_t);
}

void deserialize_entity_input(ENetPacket *packet, uint16_t &eid, float &thr, float &steer)
{
  uint8_t *ptr = packet->data;
  ptr += sizeof(uint8_t);
  eid = *(uint16_t *)(ptr);
  ptr += sizeof(uint16_t);
  thr = *(float *)(ptr);
  ptr += sizeof(float);
  steer = *(float *)(ptr);
  ptr += sizeof(float);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &ori, enet_uint32 &timeStamp)
{
  uint8_t *ptr = packet->data;
  ptr += sizeof(uint8_t);
  eid = *(uint16_t *)(ptr);
  ptr += sizeof(uint16_t);
  x = *(float *)(ptr);
  ptr += sizeof(float);
  y = *(float *)(ptr);
  ptr += sizeof(float);
  ori = *(float *)(ptr);
  ptr += sizeof(float);
  timeStamp = *(enet_uint32 *)(ptr);
}
