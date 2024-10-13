// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include <iostream>
#include <functional>
#include "raylib.h"
#include <enet/enet.h>
#include <math.h>

#include <vector>
#include "entity.h"
#include "protocol.h"

class Interpolator;

std::vector<Entity *> entities;
std::vector<Interpolator *> interpolators;
static uint16_t my_entity = invalid_entity;
// float timeSinceSnapshot = 0;
// float snapshotDelay = 1000;
constexpr enet_uint32 fixedDt = 100;
enet_uint32 lastUpdate = enet_time_get();

struct TickData
{
  float x = 0.f;
  float y = 0.f;
  float speed = 0.f;
  float ori = 0.f;
  float thr = 0.f;
  float steer = 0.f;
};

class History
{
public:
  History() {}

  void add(TickData td)
  {
    history[cur] = td;
    cur = (cur + 1) % 1000;
  }

  TickData get(size_t stepsInThePast)
  {
    size_t neededTick = cur - stepsInThePast;
    if (neededTick < 0)
      neededTick += 1000;
    if (neededTick < 0 || neededTick >= 1000)
    {
      return history[(cur + 999) % 1000];
    }

    return history[neededTick];
  }

  void set(size_t stepsInThePast, TickData td)
  {
    size_t neededTick = cur - stepsInThePast;
    if (neededTick < 0)
      neededTick += 1000;
    if (neededTick < 0 || neededTick >= 1000)
    {
      return;
    }
    history[neededTick] = td;
  }

private:
  size_t cur = 0;
  TickData history[1000];
};

class Interpolator
{
public:
  Interpolator(Entity *e)
  {
    ent = e;

    initialX = ent->x;
    initialY = ent->y;
    initialOri = ent->ori;

    targetX = ent->x;
    targetY = ent->y;
    targetOri = ent->ori;

    duration = 10000;
  }

  void updateInterpolation(float tX, float tY, float tOri, float d, enet_uint32 timeStamp)
  {
    initialX = ent->x;
    initialY = ent->y;
    initialOri = ent->ori;

    targetX = tX;
    targetY = tY;
    targetOri = tOri;

    // duration = d;
    duration = timeStamp - lastTimeStamp;
    lastTimeStamp = timeStamp;
  }

  void interpolate()
  {
    enet_uint32 currentTimeStamp = enet_time_get();

    ent->x = initialX + (targetX - initialX) * static_cast<float>(currentTimeStamp - lastTimeStamp) / duration;
    ent->y = initialY + (targetY - initialY) * static_cast<float>(currentTimeStamp - lastTimeStamp) / duration;
    ent->ori = initialOri + (targetOri - initialOri) * static_cast<float>(currentTimeStamp - lastTimeStamp) / duration;
  }

  uint16_t getEid()
  {
    return ent->eid;
  }

private:
  Entity *ent;
  enet_uint32 lastTimeStamp;
  enet_uint32 timeStamp;

  float duration;
  float initialX, initialY, initialOri;
  float targetX, targetY, targetOri;

  friend void on_snapshot(ENetPacket *packet);
};

void on_new_entity_packet(ENetPacket *packet)
{
  Entity *newEntity = new Entity();
  deserialize_new_entity(packet, *newEntity);
  // TODO: Direct adressing, of course!
  for (const Entity *e : entities)
    if (e->eid == newEntity->eid)
      return; // don't need to do anything, we already have entity
  entities.push_back(newEntity);
  interpolators.push_back(new Interpolator(newEntity));
}

void on_set_controlled_entity(ENetPacket *packet)
{
  deserialize_set_controlled_entity(packet, my_entity);
}

void on_snapshot(ENetPacket *packet)
{
  // uint16_t eid = invalid_entity;
  // float x = 0.f;
  // float y = 0.f;
  // float ori = 0.f;
  // deserialize_snapshot(packet, eid, x, y, ori);

  uint16_t eid = invalid_entity;
  float x = 0.f;
  float y = 0.f;
  float ori = 0.f;
  enet_uint32 timeStamp = 0;

  deserialize_snapshot(packet, eid, x, y, ori, timeStamp);

  // for (Interpolator *i : interpolators)
  //   if (i->getEid() == eid)
  //   {
  //     i->updateInterpolation(x, y, ori, snapshotDelay);
  //   }
  for (Interpolator *i : interpolators)
  {
    if (i->getEid() == eid)
    {
      if (i->getEid() != my_entity)
      {
        i->updateInterpolation(x, y, ori, enet_time_get(), timeStamp);
      }
      else
      {
        i->ent->x = x;
        i->ent->y = y;
        i->ent->ori = ori;
      }
    }
  }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  ENetHost *client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10131;

  ENetPeer *serverPeer = enet_host_connect(client, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }

  int width = 600;
  int height = 600;

  InitWindow(width, height, "W3 networked MIPT");

  const int scrWidth = GetMonitorWidth(0);
  const int scrHeight = GetMonitorHeight(0);
  if (scrWidth < width || scrHeight < height)
  {
    width = std::min(scrWidth, width);
    height = std::min(scrHeight - 150, height);
    SetWindowSize(width, height);
  }

  Camera2D camera = {{0, 0}, {0, 0}, 0.f, 1.f};
  camera.target = Vector2{0.f, 0.f};
  camera.offset = Vector2{width * 0.5f, height * 0.5f};
  camera.rotation = 0.f;
  camera.zoom = 10.f;

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  bool connected = false;
  History history;

  while (!WindowShouldClose())
  {
    float dt = GetFrameTime();
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        send_join(serverPeer);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
        case E_SERVER_TO_CLIENT_NEW_ENTITY:
          on_new_entity_packet(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
          on_set_controlled_entity(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SNAPSHOT:
          on_snapshot(event.packet);
          break;
        };
        break;
      default:
        break;
      };
    }
    if (my_entity != invalid_entity)
    {
      bool left = IsKeyDown(KEY_LEFT);
      bool right = IsKeyDown(KEY_RIGHT);
      bool up = IsKeyDown(KEY_UP);
      bool down = IsKeyDown(KEY_DOWN);
      // TODO: Direct adressing, of course!
      for (Entity *e : entities)
        if (e->eid == my_entity)
        {
          // Update
          float thr = (up ? 1.f : 0.f) + (down ? -1.f : 0.f);
          float steer = (left ? -1.f : 0.f) + (right ? 1.f : 0.f);

          // Send
          send_entity_input(serverPeer, my_entity, thr, steer);

          e->thr = thr;
          e->steer = steer;
          simulate_entity(*e, dt);
        }
    }

    for (Entity *e : entities)
    {
      if (e->eid == my_entity)
      {
        enet_uint32 tdt = enet_time_get() - e->timeStamp;
        if (tdt >= fixedDt)
        {
          size_t ticks;

          if (tdt / fixedDt == (tdt - 1) / fixedDt)
            ticks = tdt / fixedDt + 1;
          else
            ticks = tdt / fixedDt;

          TickData td = history.get(ticks);

          e->x = td.x;
          e->y = td.y;
          e->ori = td.ori;
          e->speed = td.speed;

          for (size_t t = ticks - 1; t > 0; --t)
          {
            simulate_entity(*e, fixedDt * 0.001);

            TickData controls = history.get(t);

            e->thr = controls.thr;
            e->steer = controls.steer;

            controls.x = e->x;
            controls.y = e->y;
            controls.ori = e->ori;
            controls.speed = e->speed;

            history.set(t, controls);
          }

          e->timeStamp = enet_time_get();
        }

        simulate_entity(*e, dt);
      }
    }

    for (Interpolator *i : interpolators)
    {
      // i->interpolate(dt);
      if (i->getEid() != my_entity)
      {
        i->interpolate();
      }
    }

    BeginDrawing();
    ClearBackground(GRAY);
    BeginMode2D(camera);

    for (const Entity *e : entities)
    {
      const Rectangle rect = {e->x, e->y, 3.f, 1.f};
      DrawRectanglePro(rect, {0.f, 0.5f}, e->ori * 180.f / PI, GetColor(e->color));
    }

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
