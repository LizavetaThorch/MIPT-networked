#include <enet/enet.h>
#include <iostream>
#include <cmath>
#include "entity.h"
#include "protocol.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include <cstring>

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer *> controlledMap;
static std::map<uint16_t, size_t> scoreMap;

static uint16_t create_random_entity()
{
  uint16_t newEid = entities.size();
  uint32_t color = 0x80ff00ff +
                   0x00440000 * (1 + rand() % 4) +
                   0x00004400 * (1 + rand() % 4) +
                   0x8000ffff * (1 + rand() % 4);
  float x = (rand() % 40 - 20) * 5.f;
  float y = (rand() % 40 - 20) * 5.f;
  Entity ent = {color, x, y, newEid, false, 0.f, 0.f, 10 + rand() % 20};
  entities.push_back(ent);
  return newEid;
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  // find max eid
  uint16_t newEid = create_random_entity();
  const Entity &ent = entities[newEid];
  scoreMap[newEid] = ent.size;

  controlledMap[newEid] = peer;

  // send info about new entity to everyone
  for (size_t i = 0; i < host->connectedPeers; ++i)
    send_new_entity(&host->peers[i], ent);
  for (size_t i = 0; i < host->connectedPeers; ++i)
  {
    // send info about controlled entity
    send_set_controlled_entity(peer, newEid);
  }
}

void on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f;
  float y = 0.f;
  size_t size;
  deserialize_entity_state(packet, eid, x, y, size);

  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.x = x;
      e.y = y;
      e.size = size;
    }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10131;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  constexpr int numAi = 10;

  for (int i = 0; i < numAi; ++i)
  {
    uint16_t eid = create_random_entity();
    entities[eid].serverControlled = true;
    controlledMap[eid] = nullptr;
    scoreMap[eid] = entities[eid].size;
  }

  uint32_t lastTime = enet_time_get();
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
        case E_CLIENT_TO_SERVER_JOIN:
          on_join(event.packet, event.peer, server);
          break;
        case E_CLIENT_TO_SERVER_STATE:
          on_state(event.packet);
          break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    for (Entity &e : entities)
    {
      if (e.serverControlled)
      {
        const float diffX = e.targetX - e.x;
        const float diffY = e.targetY - e.y;
        const float dirX = diffX > 0.f ? 1.f : -1.f;
        const float dirY = diffY > 0.f ? 1.f : -1.f;
        constexpr float spd = 50.f;
        e.x += dirX * spd * dt;
        e.y += dirY * spd * dt;
        if (fabsf(diffX) < 10.f && fabsf(diffY) < 10.f)
        {
          e.targetX = (rand() % 40 - 20) * 15.f;
          e.targetY = (rand() % 40 - 20) * 15.f;
        }
      }
    }

    for (int i = 0; i < entities.size() - 1; ++i)
    {
      for (int j = i + 1; j < entities.size(); ++j)
      {
        if (fabsf(entities[i].x + entities[i].size / 2.0f - entities[j].x - entities[j].size / 2.0f) < (entities[i].size + entities[j].size) / 2.0f && fabsf(entities[i].y + entities[i].size / 2.0f - entities[j].y - entities[j].size / 2.0f) < (entities[i].size + entities[j].size) / 2.0f)
        {
          if (entities[i].size > entities[j].size)
          {
            entities[i].size += entities[j].size / 2;
            entities[j].size /= 2;
            entities[j].x = (rand() % 40 - 20) * 15.f;
            entities[j].y = (rand() % 40 - 20) * 15.f;
          }
          else if (entities[j].size > entities[i].size)
          {
            entities[j].size += entities[i].size / 2;
            entities[i].size /= 2;
            entities[i].x = (rand() % 40 - 20) * 15.f;
            entities[i].y = (rand() % 40 - 20) * 15.f;
          }
          scoreMap[entities[i].eid] = entities[i].size;
          scoreMap[entities[j].eid] = entities[j].size;
          for (size_t i = 0; i < server->connectedPeers; ++i)
            send_scores(&server->peers[i], scoreMap);
        }
      }
    }

    for (const Entity &e : entities)
    {
      for (size_t i = 0; i < server->connectedPeers; ++i)
      {
        ENetPeer *peer = &server->peers[i];
        if (controlledMap[e.eid] != peer)
        {
          send_snapshot(peer, e);
        }
      }
    }
    // usleep(400000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}
