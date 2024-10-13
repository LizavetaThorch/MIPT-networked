#include "protocol.h"
#include "quantisation.h"
#include <cstring> // memcpy
#include <iostream>
#include "bitstream.h"

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

void send_entity_input(ENetPeer *peer, uint16_t eid, float thr, float ori)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t),
                                          ENET_PACKET_FLAG_UNSEQUENCED);
  uint8_t *ptr = packet->data;
  *ptr = E_CLIENT_TO_SERVER_INPUT;
  ptr += sizeof(uint8_t);
  memcpy(ptr, &eid, sizeof(uint16_t));
  ptr += sizeof(uint16_t);
  float4bitsQuantized thrPacked(thr, -1.f, 1.f);
  float4bitsQuantized oriPacked(ori, -1.f, 1.f);
  uint8_t thrSteerPacked = (thrPacked.packedVal << 4) | oriPacked.packedVal;
  memcpy(ptr, &thrSteerPacked, sizeof(uint8_t));
  ptr += sizeof(uint8_t);
  /*
  memcpy(ptr, &thrPacked, sizeof(uint8_t)); ptr += sizeof(uint8_t);
  memcpy(ptr, &oriPacked, sizeof(uint8_t)); ptr += sizeof(uint8_t);
  */

  enet_peer_send(peer, 1, packet);
}

typedef PackedFloat<uint16_t, 11> PositionXQuantized;
typedef PackedFloat<uint16_t, 10> PositionYQuantized;

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float ori)
{
  Bitstream pdata;
  pdata.write(E_SERVER_TO_CLIENT_SNAPSHOT);
  pdata.write(eid);

  // Testing PackedVec2 on position
  PackedVec2<uint32_t, 17, 15> packedPos({x, y}, {-16, -8}, {16, 8});
  pdata.write(packedPos.packedVal);

  uint8_t oriPacked = pack_float<uint8_t>(ori, -PI, PI, 8);
  pdata.write(oriPacked);

  ENetPacket *packet = enet_packet_create(pdata.get(), pdata.size(), ENET_PACKET_FLAG_UNSEQUENCED);

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
  uint8_t thrSteerPacked = *(uint8_t *)(ptr);
  ptr += sizeof(uint8_t);
  /*
  uint8_t thrPacked = *(uint8_t*)(ptr); ptr += sizeof(uint8_t);
  uint8_t oriPacked = *(uint8_t*)(ptr); ptr += sizeof(uint8_t);
  */
  static uint8_t neutralPackedValue = pack_float<uint8_t>(0.f, -1.f, 1.f, 4);
  static uint8_t nominalPackedValue = pack_float<uint8_t>(1.f, 0.f, 1.2f, 4);
  float4bitsQuantized thrPacked(thrSteerPacked >> 4);
  float4bitsQuantized steerPacked(thrSteerPacked & 0x0f);
  thr = thrPacked.packedVal == neutralPackedValue ? 0.f : thrPacked.unpack(-1.f, 1.f);
  steer = steerPacked.packedVal == neutralPackedValue ? 0.f : steerPacked.unpack(-1.f, 1.f);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &ori)
{
  Bitstream pdata(packet->data + 1, packet->dataLength - 1);
  pdata.read(eid);

  uint32_t packedData;
  pdata.read(packedData);
  PackedVec2<uint32_t, 17, 15> packedPos(packedData);
  Vec2 pos = packedPos.unpack({-16, -8}, {16, 8});
  x = pos.x;
  y = pos.y;

  uint8_t oriPacked;
  pdata.read(oriPacked);
  ori = unpack_float<uint8_t>(oriPacked, -PI, PI, 8);
}
