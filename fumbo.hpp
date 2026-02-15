#pragma once
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <string>
#include <unordered_map>
#include <vector>

// === FUMBO IGameState

// Abstract base class for all game states
class IGameState {
public:
  virtual ~IGameState() = default;

  virtual void Init() = 0;
  virtual void Cleanup() = 0;

  virtual void Pause() {}
  virtual void Resume() {}

  virtual void Update() = 0;
  virtual void DrawClean() = 0;
  virtual void DrawDirty() = 0;
};

// Forward declarations and enums
enum class ButtonAlign { LEFT, MIDDLE, RIGHT, TOP, BOTTOM };

// Fumbo Collision Types
namespace Fumbo {
namespace Graphic2D {

// Shape types supported by the physics engine
enum class ShapeType { Rectangle, Circle, Triangle, Polygon, Line };

// Body type determines physics behavior
enum class BodyType {
  Static, // Doesn't move, but can collide
  Dynamic // Affected by gravity and forces
};

// Collision data for contact resolution
struct CollisionContact {
  Vector2 point;     // Contact point in world space
  Vector2 normal;    // Collision normal (from A to B)
  float penetration; // Penetration depth
  bool hasCollision; // Whether collision occurred
};

// Forward declaration
class Object;

// === FUMBO Collision

// Collision detection functions
namespace Collision {

// Main collision detection dispatcher
CollisionContact CheckCollision(const Object *a, const Object *b);

// Shape-specific collision checks
CollisionContact RectangleVsRectangle(Vector2 posA, float widthA, float heightA,
                                      float rotA, Vector2 posB, float widthB,
                                      float heightB, float rotB);

CollisionContact CircleVsCircle(Vector2 posA, float radiusA, Vector2 posB,
                                float radiusB);

CollisionContact RectangleVsCircle(Vector2 rectPos, float width, float height,
                                   float rotation, Vector2 circlePos,
                                   float radius);

CollisionContact PolygonVsPolygon(const std::vector<Vector2> &vertsA,
                                  const std::vector<Vector2> &vertsB);

CollisionContact PolygonVsCircle(const std::vector<Vector2> &verts,
                                 Vector2 circlePos, float radius);

// Helper functions
Vector2 RotatePoint(Vector2 point, Vector2 origin, float angle);
std::vector<Vector2> GetRectangleVertices(Vector2 pos, float width,
                                          float height, float rotation);
Rectangle GetBoundingBox(const std::vector<Vector2> &vertices);
bool LineIntersection(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                      Vector2 *intersection = nullptr);

} // namespace Collision

// Collision layer management (32 layers max)
class CollisionLayers {
public:
  static constexpr int MAX_LAYERS = 32;

  CollisionLayers() : layerMask(0xFFFFFFFF) {} // Collide with all by default

  void SetLayer(int layer) { currentLayer = layer; }
  int GetLayer() const { return currentLayer; }

  void SetMask(uint32_t mask) { layerMask = mask; }
  uint32_t GetMask() const { return layerMask; }

  void EnableLayer(int layer) { layerMask |= (1 << layer); }
  void DisableLayer(int layer) { layerMask &= ~(1 << layer); }

  bool CanCollideWith(const CollisionLayers &other) const {
    return (layerMask & (1 << other.currentLayer)) != 0;
  }

private:
  int currentLayer = 0;
  uint32_t layerMask;
};

// 2D Physics Object with shape and rigidbody
class Object {
public:
  Object();
  ~Object() = default;

  // ===== Shape Type Configuration =====
  void SetRectangle(float width, float height);
  void SetCircle(float radius);
  void SetTriangle(Vector2 p1, Vector2 p2, Vector2 p3);
  void SetPolygon(const std::vector<Vector2> &vertices);
  void SetLine(Vector2 start, Vector2 end);

  ShapeType GetShapeType() const { return shapeType; }

  // ===== Transform =====
  void SetPosition(Vector2 pos) { position = pos; }
  Vector2 GetPosition() const { return position; }

  void SetRotation(float rot) { rotation = rot; }
  float GetRotation() const { return rotation; }

  void SetScale(float scl) { scale = scl; }
  float GetScale() const { return scale; }

  // ===== Rigidbody Properties =====
  void SetBodyType(BodyType type) { bodyType = type; }
  BodyType GetBodyType() const { return bodyType; }

  void SetVelocity(Vector2 vel) { velocity = vel; }
  Vector2 GetVelocity() const { return velocity; }

  void SetMass(float m) { mass = fmaxf(m, 0.001f); } // Prevent zero mass
  float GetMass() const { return mass; }

  void SetFriction(float f) { friction = f; }
  float GetFriction() const { return friction; }

  void SetDrag(float d) { drag = d; }
  float GetDrag() const { return drag; }

  void SetRestitution(float r) { restitution = r; }
  float GetRestitution() const { return restitution; }

  void SetGravityScale(float gs) { gravityScale = gs; }
  float GetGravityScale() const { return gravityScale; }

  // ===== Physics Simulation =====
  void ApplyForce(Vector2 force);
  void ApplyImpulse(Vector2 impulse);
  void Update(float deltaTime);

  // ===== Collision =====
  void SetTrigger(bool trigger) { isTrigger = trigger; }
  bool IsTrigger() const { return isTrigger; }

  // Collidable toggle - if false, object passes through everything
  void SetCollidable(bool collidable) { isCollidable = collidable; }
  bool IsCollidable() const { return isCollidable; }

  void SetCollisionLayers(const CollisionLayers &layers) {
    collisionLayers = layers;
  }
  CollisionLayers &GetCollisionLayers() { return collisionLayers; }
  const CollisionLayers &GetCollisionLayers() const { return collisionLayers; }

  // Check if this object is currently colliding with another
  bool IsCollidingWith(const Object *other) const;

  // ===== Shape-Specific Getters =====
  float GetWidth() const { return width; }
  float GetHeight() const { return height; }
  float GetRadius() const { return radius; }
  std::vector<Vector2> GetVertices() const;

  // Get axis-aligned bounding box (for broadphase collision detection)
  Rectangle GetAABB() const;

  // ===== Rendering =====
  void SetColor(Color c) { color = c; }
  Color GetColor() const { return color; }

  // Texture support
  void SetTexture(Texture2D tex) {
    texture = tex;
    hasTexture = true;
  }
  void ClearTexture() { hasTexture = false; }
  bool HasTexture() const { return hasTexture; }

  void SetOutline(bool outline) { isOutline = outline; }
  bool IsOutline() const { return isOutline; }

  void SetThickness(float t) { thickness = t; }
  float GetThickness() const { return thickness; }

  void Render() const;

  // ===== Debug Visualization =====
  void DrawDebug() const;

private:
  // Shape properties
  ShapeType shapeType;
  float width, height;           // Rectangle
  float radius;                  // Circle
  std::vector<Vector2> vertices; // Polygon/Triangle

  // Transform
  Vector2 position;
  float rotation;
  float scale;

  // Rigidbody
  BodyType bodyType;
  Vector2 velocity;
  Vector2 acceleration;
  float mass;
  float friction;     // 0 = frictionless, 1 = high friction
  float drag;         // Air resistance
  float restitution;  // Bounciness (0 = no bounce, 1 = perfect bounce)
  float gravityScale; // Multiplier for gravity effect

  // Collision
  bool isTrigger;
  bool isCollidable = true; // Can this object collide?
  CollisionLayers collisionLayers;

  // Visual
  Color color;
  Texture2D texture;
  bool hasTexture = false;
  bool isOutline;
  float thickness;

  // Line-specific
  Vector2 lineStart;
  Vector2 lineEnd;

  // Helper to update vertices for transform
  void UpdateVertices();
};

} // namespace Graphic2D
} // namespace Fumbo

// === FUMBO ASSETS

// Fumbo Asset Pack Types
namespace Fumbo {
namespace Assets {

// Asset pack file format
constexpr uint32_t PACK_MAGIC = 0x4B415046; // "FPAK"
constexpr uint32_t PACK_VERSION = 1;

struct PackHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t fileCount;
};

struct PackEntry {
  uint64_t nameHash;     // Hash of the filename
  uint64_t offset;       // Offset in the pack file
  uint64_t size;         // Size of encrypted data
  uint64_t originalSize; // Size of original data
  char filename[256];    // Original filename for debugging
};

// Normalize path separators (Windows backslash to forward slash)
inline std::string NormalizePath(const std::string &path) {
  std::string normalized = path;
  for (char &c : normalized) {
    if (c == '\\')
      c = '/';
  }
  return normalized;
}

// Simple hash function for filenames
inline uint64_t HashString(const std::string &str) {
  // Normalize path before hashing to ensure Windows/Linux compatibility
  std::string normalized = NormalizePath(str);
  uint64_t hash = 5381;
  for (char c : normalized) {
    hash = ((hash << 5) + hash) + static_cast<uint64_t>(c);
  }
  return hash;
}

class AssetPack {
public:
  AssetPack() : loaded(false) {}

  // Load a pack file
  bool Load(const std::string &packPath);

  // Check if pack is loaded
  bool IsLoaded() const { return loaded; }

  // Check if asset exists in pack
  bool HasAsset(const std::string &assetPath) const;

  // Load asset data (decrypted)
  std::vector<uint8_t> LoadAsset(const std::string &assetPath) const;

  // Get original size of asset
  size_t GetAssetSize(const std::string &assetPath) const;

  // Unload pack
  void Unload();

private:
  bool loaded;
  std::string packFilePath;
  std::unordered_map<uint64_t, PackEntry> entries;
};

} // namespace Assets
} // namespace Fumbo

// === FUMBO SHADER

// Fumbo Fade Effects
class FadeEffect {
public:
  bool active = false;
  int group = 0;

  Texture2D texture{};
  std::string text;
  Font font{};
  bool isText = false;

  Vector2 pos{};    // base position in UI coordinates
  Vector2 size{};   // base size for texture
  float fontSize{}; // base font size for text

  float duration = 1.0f;
  float timer = 0.0f;
  bool fadeIn = true;

  void Start(Texture2D tex, Vector2 pos, Vector2 size, float duration,
             bool fadeIn = true, int group = 0);
  void Start(const std::string &text, Font font, Vector2 pos, float fontSize,
             float duration, bool fadeIn = true, int group = 0);
  void Draw();
  void Reset() {
    timer = 0.0f;
    active = false;
  }
  void Reverse();
  bool IsActive() const;
};

class FadeManager {
private:
  std::vector<FadeEffect> fades;

public:
  FadeManager(int maxFades = 10) { fades.resize(maxFades); }
  FadeEffect *AddFade(Texture2D tex, Vector2 pos, Vector2 size, float duration,
                      bool fadeIn = true, int group = 0);
  FadeEffect *AddFade(const std::string &text, Font font, Vector2 pos,
                      float fontSize, float duration, bool fadeIn = true,
                      int group = 0);
  void Draw(int group = -1);
  void DrawExcept(int excludedGroup);
  void RemoveGroup(int group);
  void Clear();
  void ResetFade(FadeEffect *f);
  void ResetGroup(int groupID);
  void ResetAll();
  void ReverseFade(FadeEffect *f) {
    if (f)
      f->Reverse();
  }
  void ReverseGroup(int groupID) {
    for (auto &f : fades)
      if (f.group == groupID)
        f.Reverse();
  }
  bool IsGroupActive(int groupID) const;
  bool IsGroupFinished(int groupID) const;
};

// === FUMBO UI

// Fumbo UI Types
namespace Fumbo {
namespace UI {

// Text alignment options
enum class TextAlign { LEFT, CENTER, RIGHT };

// Message box animation types
enum class BoxAnimation { NONE, FADE, SLIDE_UP, SLIDE_DOWN };

// Message box style configuration
struct MessageBoxStyle {
  // Background
  Color backgroundColor = Color{0, 0, 0, 200};
  Texture2D backgroundTexture = {0};
  bool useTexture = false;
  bool useNinePatch = false; // 9-slice scaling for texture

  // NinePatch settings (border sizes for 9-slice)
  int ninePatchLeft = 16;
  int ninePatchRight = 16;
  int ninePatchTop = 16;
  int ninePatchBottom = 16;

  // Border
  Color borderColor = WHITE;
  float borderThickness = 2.0f;
  float borderRounding = 0.0f;

  // Padding (space between box edge and text)
  float paddingTop = 20.0f;
  float paddingBottom = 20.0f;
  float paddingLeft = 20.0f;
  float paddingRight = 20.0f;

  // Shadow
  bool enableShadow = false;
  Vector2 shadowOffset = {4.0f, 4.0f};
  Color shadowColor = Color{0, 0, 0, 100};

  // Animation
  BoxAnimation animation = BoxAnimation::NONE;
  float animationDuration = 0.3f;
  float animationProgress = 1.0f; // 0.0 to 1.0
};

// Text style configuration
struct TextStyle {
  TextAlign alignment = TextAlign::LEFT;
  float lineHeightMultiplier = 1.2f;

  // Text shadow
  bool enableShadow = false;
  Vector2 shadowOffset = {2.0f, 2.0f};
  Color shadowColor = Color{0, 0, 0, 150};

  // Text outline
  bool enableOutline = false;
  float outlineThickness = 1.0f;
  Color outlineColor = BLACK;
};

struct SliderConfig {
  // Colors
  Color trackColor = LIGHTGRAY;
  Color progressColor = SKYBLUE;
  Color knobColor = DARKGRAY;
  Color outlineColor = DARKGRAY;

  // Dimensions
  float knobWidth = 20.0f;
  float knobHeight = 0.0f;  // 0 = match bounds height
  float trackHeight = 0.0f; // 0 = match bounds height
  float outlineThickness = 1.0f;

  // Textures (Optional, if .id != 0 they are used)
  Texture2D trackTexture = {0};
  Texture2D progressTexture = {0};
  Texture2D knobTexture = {0};
};

} // namespace UI
} // namespace Fumbo

// === Fumbo Graphics
namespace Fumbo {
namespace Graphic2D {
// Drawing helpers
void DrawText(const std::string &text, Vector2 basePos, Font font,
              int baseFontSize, Color color);

void DrawTexture(Texture2D texture, Vector2 basePos, Vector2 baseSize,
                 float rotation = 0.0f, Color tint = WHITE);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest,
                    Vector2 origin, float rotation, Color tint);

// Dynamic Shapes Drawing
void DrawPixel(int posX, int posY, Color color);
void DrawPixelV(Vector2 position, Color color);
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY,
              Color color);
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawLineStrip(const Vector2 *points, int pointCount, Color color);
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawCircle(int centerX, int centerY, float radius, Color color);
void DrawCircleSector(Vector2 center, float radius, float startAngle,
                      float endAngle, int segments, Color color);
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle,
                           float endAngle, int segments, Color color);
void DrawCircleGradient(int centerX, int centerY, float radius, Color inner,
                        Color outer);
void DrawCircleV(Vector2 center, float radius, Color color);
void DrawCircleLines(int centerX, int centerY, float radius, Color color);
void DrawCircleLinesV(Vector2 center, float radius, Color color);
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV,
                 Color color);
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV,
                      Color color);
void DrawRing(Vector2 center, float innerRadius, float outerRadius,
              float startAngle, float endAngle, int segments, Color color);
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius,
                   float startAngle, float endAngle, int segments, Color color);
void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleV(Vector2 position, Vector2 size, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation,
                      Color color);
void DrawRectangleGradientV(int posX, int posY, int width, int height,
                            Color top, Color bottom);
void DrawRectangleGradientH(int posX, int posY, int width, int height,
                            Color left, Color right);
void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft,
                             Color topRight, Color bottomRight);
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
void DrawRectangleRounded(Rectangle rec, float roundness, int segments,
                          Color color);
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments,
                               Color color);
void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments,
                                 float lineThick, Color color);
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleFan(const Vector2 *points, int pointCount, Color color);
void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color);
void DrawPoly(Vector2 center, int sides, float radius, float rotation,
              Color color);
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation,
                   Color color);
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation,
                     float lineThick, Color color);

// Dynamic Splines
void DrawSplineLinear(const Vector2 *points, int pointCount, float thick,
                      Color color);
void DrawSplineBasis(const Vector2 *points, int pointCount, float thick,
                     Color color);
void DrawSplineCatmullRom(const Vector2 *points, int pointCount, float thick,
                          Color color);
void DrawSplineBezierQuadratic(const Vector2 *points, int pointCount,
                               float thick, Color color);
void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick,
                           Color color);
void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, float thick, Color color);
void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                            float thick, Color color);
void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
                                 float thick, Color color);
void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3,
                                      float thick, Color color);
void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3,
                                  Vector2 p4, float thick, Color color);

// Utilities
Texture2D CaptureScreenToTexture();
void DrawBackground(Texture2D);
} // namespace Graphic2D
} // namespace Fumbo

// === Fumbo Physics
namespace Fumbo {
namespace Graphic2D {

// Raycast result structure
struct RaycastHit {
  Object *object;
  Vector2 point;
  Vector2 normal;
  float distance;
  bool hit;
};

// Physics manager (Singleton)
class Physics {
public:
  static Physics &Instance() {
    static Physics instance;
    return instance;
  }

  // Global physics settings
  void SetGravity(Vector2 newGravity) { gravity = newGravity; }
  Vector2 GetGravity() const { return gravity; }

  void SetFixedTimeStep(float timeStep) { fixedTimeStep = 1.0f / timeStep; }
  float GetFixedTimeStep() const { return fixedTimeStep; }

  void SetIterations(int newIterations) { iterations = newIterations; }
  int GetIterations() const { return iterations; }

  // Object management
  void AddObject(Object *object);
  void RemoveObject(Object *object);
  void Clear();

  const std::vector<Object *> &GetObjects() const { return objects; }

  // Physics simulation
  void Update(float deltaTime);

  // Raycasting
  RaycastHit Raycast(Vector2 origin, Vector2 direction, float maxDistance);
  std::vector<RaycastHit> RaycastAll(Vector2 origin, Vector2 direction,
                                     float maxDistance);

  // Debug rendering
  void SetDebugDraw(bool enabled) { debugDraw = enabled; }
  bool IsDebugDrawEnabled() const { return debugDraw; }
  void DrawDebug() const;

private:
  Physics();
  ~Physics() = default;
  Physics(const Physics &) = delete;
  Physics &operator=(const Physics &) = delete;

  Vector2 gravity;
  float fixedTimeStep;
  float accumulator;
  int iterations;
  bool debugDraw;

  std::vector<Object *> objects;

  // Physics step
  void Step(float deltaTime);
  void ApplyGravity(float deltaTime);
  void ResolveCollisions();
  void ResolveCollision(Object *objectA, Object *objectB,
                        const CollisionContact &contact);
};

} // namespace Graphic2D
} // namespace Fumbo

// === Fumbo Audio
namespace Fumbo {
namespace Audio {
enum class AudioType { SOUND, MUSIC };

class AudioManager {
public:
  static AudioManager &Instance() {
    static AudioManager instance;
    return instance;
  }

  void Cleanup();
  void Update(); // Required for looping music manually

  // Sound Effects
  void PlaySound(const std::string &id);
  void StopSound(const std::string &id);

  // Music
  // loopStart: offset in seconds to loop back to when music ends.
  // If loopStart < 0, it loops from beginning.
  // If looping is false, it plays once.
  void PlayMusic(const std::string &id, bool loop = true,
                 float loopStart = 0.0f);
  void StopMusic(const std::string &id);
  void StopMusicFade(float duration = 1.0f); // Fade out current music
  void StopAllMusic();

  // Resource Loading (Manual loading if desired, or auto-load via Play)
  bool LoadAudio(const std::string &id, const std::string &path,
                 AudioType type);
  void UnloadAudio(const std::string &id);

  // Volume Control (0.0f to 1.0f)
  void SetMasterVolume(float vol);
  void SetMusicVolume(float vol);
  void SetSoundVolume(float vol);

  float GetMasterVolume() const { return masterVol; }
  float GetMusicVolume() const { return musicVol; }
  float GetSoundVolume() const { return soundVol; }

private:
  AudioManager() = default;
  ~AudioManager() = default;

  std::map<std::string, ::Sound> sounds;
  std::map<std::string, ::Music> musics;

  // Track active music state for custom looping
  struct MusicState {
    std::string id;
    bool loop;
    float loopStart;
    bool active;

    // Fading
    bool fadingOut;
    float fadeDuration;
    float fadeTimer;
    float startVol;
  };

  MusicState currentMusic = {"", false, 0.0f, false, false, 0.0f, 0.0f, 1.0f};

  float masterVol = 1.0f;
  float musicVol = 1.0f;
  float soundVol = 1.0f;
};
} // namespace Audio
} // namespace Fumbo

// === FUMBO BUTTON

// Fumbo UI Classes
namespace Fumbo {
namespace UI {

class Button {
public:
  ~Button();
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;
  Button(Button &&other) noexcept;
  Button &operator=(Button &&other) noexcept;

  Button() = default;

  Button(Texture2D texture, Sound hoverSound = {}, Sound clickSound = {});

  void Draw();

  void AddText(const std::string &text, Font font, int fontSize,
               Color color = BLACK);

  void AlignText(ButtonAlign vertical = ButtonAlign::MIDDLE,
                 ButtonAlign horizontal = ButtonAlign::MIDDLE);

  void HoveredColor(Color color);
  void IdleColor(Color color);
  void DisabledColor(Color color);
  void SetPosition(Vector2 Position);
  void SetBounds(Rectangle bounds);
  void SetBounds(float x, float y, float w, float h);

  void TextOffsetX(float offset);
  void TextOffsetY(float offset);
  void TextOffsetXY(float offsetX, float offsetY);

  Vector2 Position;
  void SetInteractable(bool interactable);
  void SetWorldSpace(bool worldSpace);
  bool IsPressed() const;
  bool IsReleased() const;
  bool IsHover() const;

private:
  void Update(Camera2D *camera = nullptr);
  Rectangle uiBounds{};
  Texture2D texture{};
  std::string text;
  Font font{};
  int baseFontSize{};
  Color m_textColor = BLACK;

  Sound hoverSound{};
  Sound clickSound{};

  ButtonAlign horizontalAlign = ButtonAlign::MIDDLE;
  ButtonAlign verticalAlign = ButtonAlign::MIDDLE;

  bool hovered = false;
  Color m_hoveredColor = WHITE;
  Color m_idleColor = {200, 200, 200, 255};
  Color m_disabledColor = Fade(m_idleColor, 0.5f);
  float m_textOffsetX = 0;
  float m_textOffsetY = 0;

  // Caching
  RenderTexture2D m_cacheTexture = {0};
  bool m_isDirty = true;
  bool m_interactable = true;
  int m_lastWidth = 0;
  int m_lastHeight = 0;
  Camera2D *m_camera = nullptr;
  bool m_worldSpace = false;

  // State tracking for auto-update
  bool m_isPressed = false;
  bool m_isReleased = false;
  mutable double m_lastUpdateTime = -1.0;
};

class Slider {
public:
  Slider() = default;
  Slider(float min, float max, float initialValue);

  // Returns true if value changed
  bool Update(Rectangle bounds);
  void Draw(Rectangle bounds);

  void SetValue(float value);
  float GetValue() const;

  void SetStyle(const SliderConfig &config);

private:
  float m_min = 0.0f;
  float m_max = 1.0f;
  float m_value = 0.5f;

  bool m_dragging = false;

  SliderConfig m_config;
};

// === FUMBO VN

class VisualNovel {
public:
  VisualNovel(const std::string &text, float charsPerSecond);

  void Update(float deltaTime);
  void Draw(Font font, Vector2 startPos, float fontSize, float spacing,
            float maxX, float maxY, Color color);

  void Reset();
  void Clear(); // Unload scene (clear characters/text)
  void Skip();

  // New Features
  void SetSpeaker(const std::string &name, Color color = WHITE);
  void SetTypingSound(Sound sound);
  void SetText(const std::string &text); // Helper to reset with new text
  bool IsComplete() const; // Check if typing animation is complete

  // Character Sprites
  void AddCharacter(const std::string &name, Texture2D sprite,
                    float scale = 1.0f);
  void SetCharacterPosition(const std::string &name,
                            Vector2 pos); // Instant Teleport
  void MoveCharacterPosition(const std::string &name, Vector2 targetPos,
                             float duration); // Animated Move

  // Fading
  void SetCharacterAlpha(const std::string &name, float alpha);
  void FadeCharacter(const std::string &name, float targetAlpha,
                     float duration);
  void FadeInCharacter(const std::string &name, float duration);
  void FadeOutCharacter(const std::string &name, float duration);

  void DrawSprites(); // Call before drawing text box

  // Message Box Customization
  void SetMessageBoxStyle(const MessageBoxStyle &style);
  void SetMessageBoxTexture(Texture2D texture, bool useNinePatch = false);
  void SetMessageBoxBounds(Rectangle bounds); // Set position and size
  void EnableMessageBox(bool enable);

  // Text Customization
  void SetTextStyle(const TextStyle &style);
  void SetTextAlignment(TextAlign alignment);

  // Complete rendering (sprites + message box + text)
  void DrawComplete(Font font, float fontSize, float spacing,
                    Color textColor = WHITE);

private:
  // Internal usage to hold process state
  struct Word {
    std::string text;
    Color color;
    bool isTag;
  };

  struct Character {
    Texture2D sprite;
    Vector2 position;
    float scale;
    Color tint;

    // Animation
    bool isMoving;
    Vector2 startPos;
    Vector2 targetPos;
    float moveTimer;
    float moveDuration;

    // Fading
    bool isFading;
    float fadeTimer;
    float fadeDuration;
    float startAlpha;
    float targetAlpha;
  };

  std::map<std::string, Character> m_characters;

  std::string m_rawText;
  std::vector<std::string> m_wrappedLines;

  // Speaker
  std::string m_speakerName;
  Color m_speakerColor = WHITE;

  // Audio
  Sound m_typeSound = {0};
  float m_soundTimer = 0.0f;

  // State
  int m_visibleChars;
  float m_timer;
  float m_charsPerSecond;

  // Message box and text styling
  MessageBoxStyle m_boxStyle;
  TextStyle m_textStyle;
  Rectangle m_boxBounds = {50, 500, 1180, 200}; // Default bounds
  bool m_enableMessageBox = false;

  void RecalculateWrapping(Font font, float fontSize, float spacing,
                           float maxX);
  bool m_wrappingDone = false;
};

} // namespace UI
} // namespace Fumbo

// === Fumbo Utils
namespace Fumbo {
namespace Utils {
constexpr float UI_WIDTH = 1280.0f;
constexpr float UI_HEIGHT = 720.0f;

// UI scale
Vector2 GetUIScale();

// Coordinate helpers
Vector2 CenterPosX(Vector2 objsize);
Vector2 CenterPosY(Vector2 objsize);
Vector2 CenterPosXY(Vector2 objsize);
Rectangle UISpaceToScreen(Rectangle ui);
void DrawPixelRuler(int spacing = 50, Font font = {});
bool MoveTowards(Vector2 &current, Vector2 target, float maxDistanceDelta);
void Camera2DFollow(Camera2D *camera, Rectangle targetRect, float offsetx = 0,
                    float offsety = 0, float smoothness = 0.0f);
void Camera2DFollow(Camera2D *camera, Vector2 targetCenter, float offsetx = 0,
                    float offsety = 0, float smoothness = 0.0f);
void MakeSolidColor(Texture2D texture, Color color);
Shader MakeSolidColorShader(Color color);

// Cached versions for performance - reuse resources across frames
Texture2D DrawMask(RenderTexture2D screen, RenderTexture2D &renderTarget,
                   Shader &maskShader, Color canvasColor = WHITE,
                   Color maskColor = BLACK, bool forceRecreate = false);
Texture2D ApplyRadialBlur(Texture2D source, RenderTexture2D &canvas,
                          Shader &shader, float blurValue = 0.1f,
                          Vector2 position = {UI_WIDTH / 2, UI_HEIGHT / 2},
                          bool forceRecreate = false);

// Internal / private-like helpers
namespace Internal {
void GameSettings();
}
} // namespace Utils
} // namespace Fumbo

// Fumbo Assets
namespace Fumbo {
namespace Assets {
// Add asset packs to use (call for each pack file at startup)
void AddAssetPack(const std::string &packPath);

// Get all asset packs
const std::vector<std::unique_ptr<AssetPack>> &GetAssetPacks();

// Asset Loading Wrappers (automatically check pack first)
Texture2D LoadTexture(const std::string &fileName);
Image LoadImage(const std::string &fileName);
Font LoadFont(const std::string &fileName, int fontSize);
Sound LoadSound(const std::string &fileName);
Music LoadMusic(const std::string &fileName);
Texture2D CheckedTexture();
Image CheckedImage();
} // namespace Assets
} // namespace Fumbo

// Fumbo Video
namespace Fumbo {
namespace Video {

class VideoPlayer {
public:
  bool Init();
  bool Play(const char *path, bool skippable = false);
  void Shutdown();

private:
  mpv_handle *mpv = nullptr;
  mpv_render_context *ctx = nullptr;

  Texture2D texture = {0};
  unsigned char *frameBuffer = nullptr;
  int width = 0;
  int height = 0;
};

} // namespace Video
} // namespace Fumbo

// === Fumbo Shader Manager
namespace Fumbo {

class ShaderManager {
public:
  static ShaderManager &Instance() {
    static ShaderManager instance;
    return instance;
  }

  void Init(int width, int height);
  void Cleanup();

  void BeginBlurMode(float radius);
  void EndBlurMode();

  // Blur Pass: Compositing multiple objects
  void BeginBlurPass();
  void EndBlurPass(float radius, Vector2 pos = {0, 0});

  // Draw content with blur shader applied
  void DrawBlur(Texture2D texture, Vector2 pos, float radius);

  FadeManager &GetFader() { return fader; }

private:
  ShaderManager() = default;
  ~ShaderManager() = default;

  Shader blurShader = {0};
  int locRenderWidth = -1;
  int locRenderHeight = -1;
  int locRadius = -1;

  RenderTexture2D blurTarget = {0};
  bool blurPassActive = false;

  FadeManager fader;
};

} // namespace Fumbo

// Fumbo

// === Fumbo Engine
namespace Fumbo {

class Engine {
public:
  static Engine &Instance() {
    static Engine instance;
    return instance;
  }
  void Init(int width = 1280, int height = 720,
            const std::string &title = "Fumbo Engine", double targetFPS = 0);
  // Frame Limiter
  void LimitFPS(double fps);
  void SetVSync(bool enabled);

  // Debug / UI
  void DrawFPS(int x, int y);
  void DrawFPS(int x, int y, Font font, int fontSize, Color color);

  void Run(std::shared_ptr<IGameState> initialState);
  void Cleanup();
  void Quit();

  // State Management
  void ChangeState(std::shared_ptr<IGameState> newState);

  // Engine Services
  FadeManager &GetFader() { return ShaderManager::Instance().GetFader(); }

  ShaderManager &GetShaderManager() { return ShaderManager::Instance(); }

  Audio::AudioManager &GetAudioManager() {
    return Audio::AudioManager::Instance();
  }

  Graphic2D::Physics &GetPhysics() { return Graphic2D::Physics::Instance(); }

  // Global Accessors (Wrappers around Raylib or Utils)
  int GetWidth() const;
  int GetHeight() const;

  // Clean Layer Management
  void InvalidateCleanLayer() { m_cleanIsInvalid = true; }
  bool IsCleanLayerInvalid() const { return m_cleanIsInvalid; }

  // Global Overlay System
  using OverlayCallback = std::function<void()>;
  void AddGlobalOverlay(OverlayCallback cb);

private:
  Engine() = default;
  ~Engine() = default;
  Engine(const Engine &) = delete;
  Engine &operator=(const Engine &) = delete;

  bool running = true;
  std::shared_ptr<IGameState> currentState;
  std::shared_ptr<IGameState> nextState;
  bool stateChangePending = false;
  std::vector<OverlayCallback> m_globalOverlays;

  // Clean Layer Caching
  RenderTexture2D m_cleanTexture = {0};
  bool m_cleanIsInvalid = true;

  void Update();
  void Draw();
  static void SetFumboIcon();
};

// Convenience helper to avoid breaking changes in user code
inline Engine &Instance() { return Engine::Instance(); }

} // namespace Fumbo
