#include "../../fumbo.hpp"
#include "raylib.h"

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
const char *solidShader = R"(
#version 100
precision mediump float;
varying vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec4 solidColor; // The global color you want to apply

void main()
{
    // Get the alpha value from the texture
    float alpha = texture2D(texture0, fragTexCoord).a;
    
    // Replace RGB with solidColor, keep original texture Alpha
    gl_FragColor = vec4(solidColor.rgb, solidColor.a * alpha);
}
)";

// Particle flow shader stub for mobile (not implemented)
const char *particleFlowShader = R"(
#version 100
precision mediump float;
varying vec2 fragTexCoord;
void main()
{
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)";

const char *radialBlurShader = R"(
#version 100
precision mediump float;
varying vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec2 center;        // Center of blur (0.0 to 1.0)
uniform float blurStrength; // How strong the blur is

void main()
{
    vec4 color = vec4(0.0);
    vec2 dir = fragTexCoord - center;
    float dist = length(dir);
    dir = normalize(dir);
    
    // Sample along the ray from center
    const int samples = 32; // More samples = smoother but slower
    for (int i = 0; i < 32; i++)
    {
        float offset = (float(i) / float(samples)) * blurStrength;
        vec2 samplePos = fragTexCoord - dir * offset;
        color += texture2D(texture0, samplePos);
    }
    
    gl_FragColor = color / float(samples);
}
)";
#else
const char *solidShader = R"(
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec4 solidColor; // The global color you want to apply

void main()
{
    // Get the alpha value from the texture
    float alpha = texture(texture0, fragTexCoord).a;
    
    // Replace RGB with solidColor, keep original texture Alpha
    finalColor = vec4(solidColor.rgb, solidColor.a * alpha);
}
)";

const char *radialBlurShader = R"(
#version 330
in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec2 center;        // Center of blur (0.0 to 1.0)
uniform float blurStrength; // How strong the blur is
out vec4 finalColor;
void main()
{
    vec4 color = vec4(0.0);
    vec2 dir = fragTexCoord - center;
    float dist = length(dir);
    dir = normalize(dir);
    
    // Sample along the ray from center
    int samples = 32; // More samples = smoother but slower
    for (int i = 0; i < samples; i++)
    {
        float offset = (float(i) / float(samples)) * blurStrength;
        vec2 samplePos = fragTexCoord - dir * offset;
        color += texture(texture0, samplePos);
    }
    
    finalColor = color / float(samples);
}
)";

// ===== Particle Trail Shader (Desktop) =====
// Optimised stochastic mouse trail: magenta → purple → blue pointillist particles.
// Per-pixel hash (no cell grid = zero cutting artifacts). 32-point trail max.
const char *particleFlowShader = R"(
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform float uTime;
uniform vec2  uResolution;
uniform vec2  uTrail[32];   // trail positions in normalised UV (0..1, Y-down)
uniform int   uTrailCount;  // number of active trail points

// Fast ALU hash — no trig
float hash21(vec2 p) {
    p = fract(p * vec2(443.897, 441.423));
    p += dot(p, p.yx + 19.19);
    return fract((p.x + p.y) * p.x);
}

// Magenta → purple → blue
vec3 palette(float t) {
    t = clamp(t, 0.0, 1.0);
    vec3 a = vec3(0.90, 0.20, 0.60);
    vec3 b = vec3(0.55, 0.12, 0.85);
    vec3 c = vec3(0.18, 0.25, 0.92);
    return t < 0.5 ? mix(a, b, t * 2.0) : mix(b, c, (t - 0.5) * 2.0);
}

void main()
{
    // gl_FragCoord gives real pixel coords (fragTexCoord is invalid for DrawRectangle)
    // Flip Y: OpenGL is bottom-up, Raylib mouse/screen is top-down
    vec2 uv = vec2(gl_FragCoord.x, uResolution.y - gl_FragCoord.y) / uResolution;
    float aspect = uResolution.x / uResolution.y;
    vec2 pos = vec2(uv.x * aspect, uv.y);

    float minDist = 1e6;
    float bestAge = 1.0;
    int count = min(uTrailCount, 32);

    // --- Closest distance to trail polyline (31 segments max) ---
    for (int i = 0; i < 31; i++) {
        if (i >= count - 1) break;
        vec2 a  = vec2(uTrail[i].x * aspect, uTrail[i].y);
        vec2 b  = vec2(uTrail[i + 1].x * aspect, uTrail[i + 1].y);
        vec2 ab = b - a;
        float len2 = dot(ab, ab);
        float t = len2 > 1e-8 ? clamp(dot(pos - a, ab) / len2, 0.0, 1.0) : 0.0;
        float d = length(pos - (a + t * ab));
        if (d < minDist) {
            minDist = d;
            bestAge = (float(i) + t) / float(max(count - 1, 1));
        }
    }

    // --- Per-pixel stochastic noise (no grid cells = no clipping) ---
    float noise = hash21(floor(uv * uResolution));

    // Trail width: tight at head, hair-thin at tail
    float trailW = mix(0.018, 0.002, bestAge);
    float mask   = smoothstep(trailW, 0.0, minDist);

    // Density: thick near head, sparse at tail
    float thresh = mix(0.20, 0.82, bestAge);
    float particle = step(thresh, noise);

    vec3 color = palette(bestAge) * mask * particle;

    // --- Subtle core glow along trail ---
    float gw   = mix(0.007, 0.001, bestAge);
    float glow = exp(-minDist * minDist / (gw * gw)) * 0.10;
    color += palette(bestAge) * glow * (1.0 - bestAge * 0.6);

    // --- Compact head glow ---
    if (count > 0) {
        vec2  head = vec2(uTrail[0].x * aspect, uTrail[0].y);
        float hd   = length(pos - head);
        color += vec3(0.95, 0.50, 0.78) * exp(-hd * hd * 3000.0) * 0.7;
        color += vec3(0.50, 0.15, 0.60) * exp(-hd * hd * 600.0)  * 0.25;
    }

    finalColor = vec4(color, 1.0);
}
)";
#endif

namespace Fumbo::Shaders {
void MakeSolidColor(Texture2D texture, Color color) {
  Color *pixels = LoadImageColors(LoadImageFromTexture(texture));

  for (int i = 0; i < texture.width * texture.height; i++) {
    if (pixels[i].a > 0) { // If pixel is not transparent
      pixels[i] = color;
    }
  }
  UpdateTexture(texture, pixels);
  UnloadImageColors(pixels);
}

Shader MakeSolidColorShader(Color color) {
  Shader solid = LoadShaderFromMemory(0, solidShader);
  int solidLoc = GetShaderLocation(solid, "solidColor");
  float colorfloat[4] = {(float)color.r / 255, (float)color.g / 255,
                         (float)color.b / 255, (float)color.a / 255};
  SetShaderValue(solid, solidLoc, colorfloat, SHADER_UNIFORM_VEC4);
  return solid;
}

Texture2D DrawMask(RenderTexture2D screen, RenderTexture2D &renderTarget,
                   Shader &maskShader, Color canvasColor, Color maskColor,
                   bool forceRecreate) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Check if we need to recreate resources
  bool needsRecreate = forceRecreate || renderTarget.id == 0 ||
                       renderTarget.texture.width != screenWidth ||
                       renderTarget.texture.height != screenHeight;

  if (needsRecreate) {
    // Cleanup old resources
    if (renderTarget.id != 0) {
      UnloadRenderTexture(renderTarget);
    }
    if (maskShader.id != 0) {
      UnloadShader(maskShader);
    }

    // Create new resources
    renderTarget = LoadRenderTexture(screenWidth, screenHeight);
    maskShader = MakeSolidColorShader(maskColor);
  }

  // Render to texture
  BeginTextureMode(renderTarget);
  ClearBackground(canvasColor);
  BeginShaderMode(maskShader);
  DrawTexturePro(screen.texture,
                 (Rectangle){0, 0, (float)screenWidth, -(float)screenHeight},
                 (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                 (Vector2){0, 0}, 0.0f, WHITE);

  EndShaderMode();
  EndTextureMode();

  // Return the texture (no flip needed, will handle in shader/drawing)
  return renderTarget.texture;
}

Texture2D ApplyRadialBlur(Texture2D source, RenderTexture2D &canvas,
                          Shader &shader, float blurValue, Vector2 position,
                          bool forceRecreate) {
  // Check if we need to recreate resources
  bool needsRecreate = forceRecreate || canvas.id == 0 || shader.id == 0 ||
                       canvas.texture.width != source.width ||
                       canvas.texture.height != source.height;

  // Static variables to cache shader uniform locations
  static int centerLoc = -1;
  static int blurStrengthLoc = -1;
  static unsigned int cachedShaderId = 0;

  if (needsRecreate) {
    // Cleanup old resources
    if (canvas.id != 0) {
      UnloadRenderTexture(canvas);
    }
    if (shader.id != 0) {
      UnloadShader(shader);
    }

    // Create new resources
    canvas = LoadRenderTexture(source.width, source.height);
    shader = LoadShaderFromMemory(NULL, radialBlurShader);

    // Cache shader uniform locations for new shader
    centerLoc = GetShaderLocation(shader, "center");
    blurStrengthLoc = GetShaderLocation(shader, "blurStrength");
    cachedShaderId = shader.id;
  } else if (shader.id != cachedShaderId) {
    // Shader was recreated externally, update cached locations
    centerLoc = GetShaderLocation(shader, "center");
    blurStrengthLoc = GetShaderLocation(shader, "blurStrength");
    cachedShaderId = shader.id;
  }

  // Apply blur
  BeginTextureMode(canvas);
  ClearBackground(BLANK);
  BeginShaderMode(shader);

  // Normalize position to 0-1 range based on actual screen dimensions
  Vector2 normalizedPos = {position.x / (float)GetScreenWidth(),
                           position.y / (float)GetScreenHeight()};

  // Set shader parameters using cached locations
  SetShaderValue(shader, centerLoc, &normalizedPos, SHADER_UNIFORM_VEC2);
  SetShaderValue(shader, blurStrengthLoc, &blurValue, SHADER_UNIFORM_FLOAT);

  DrawTexturePro(source,
                 (Rectangle){0, 0, (float)source.width, (float)source.height},
                 (Rectangle){0, 0, (float)source.width, (float)source.height},
                 (Vector2){0, 0}, 0.0f, WHITE);
  EndShaderMode();
  EndTextureMode();

  // Return the texture directly (no image flip needed)
  return canvas.texture;
}
} // namespace Fumbo::Shaders
