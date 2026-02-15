#include "graphics.hpp"

#include <algorithm>

// Helper to scale a point array
static void ScalePoints(const Vector2* points, int pointCount, Vector2* outPoints, Vector2 scale) {
  for (int i = 0; i < pointCount; i++) {
    outPoints[i].x = points[i].x * scale.x;
    outPoints[i].y = points[i].y * scale.y;
  }
}

namespace Fumbo {
  namespace Graphic2D {
    using Fumbo::Utils::GetUIScale;
    using Fumbo::Utils::UI_HEIGHT;
    using Fumbo::Utils::UI_WIDTH;

    Texture2D CaptureScreenToTexture() {
      Image screenImage = LoadImageFromScreen();
      Texture2D texture = LoadTextureFromImage(screenImage);
      UnloadImage(screenImage);
      return texture;
    }

    void DrawText(const std::string& text, Vector2 basePos, Font font, int baseFontSize, Color color) {
      Vector2 scale = GetUIScale();
      float fontSize = baseFontSize * scale.y;
      Vector2 position = {basePos.x * scale.x, basePos.y * scale.y};

      DrawTextEx(font, text.c_str(), position, fontSize, 1.0f * scale.x, color);
    }

    void DrawTexture(Texture2D texture, Vector2 basePos, Vector2 baseSize, float rotation, Color tint) {
      Vector2 scale = GetUIScale();

      Rectangle sourceRect = {0, 0, (float)texture.width, (float)texture.height};

      Rectangle destRect = {basePos.x * scale.x, basePos.y * scale.y, baseSize.x * scale.x,
                          baseSize.y * scale.y};

      Vector2 origin = {0, 0};

      ::DrawTexturePro(texture, sourceRect, destRect, origin, rotation, tint);
    }

    void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin,
                        float rotation, Color tint) {
      Vector2 scale = GetUIScale();
      Rectangle destRect = {dest.x * scale.x, dest.y * scale.y, dest.width * scale.x, dest.height * scale.y};
      Vector2 originVec = {origin.x * scale.x, origin.y * scale.y};
      ::DrawTexturePro(texture, source, destRect, originVec, rotation, tint);
    }

    // Dynamic Shapes

    void DrawPixel(int posX, int posY, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawPixel((int)(posX * scale.x), (int)(posY * scale.y), color);
    }

    void DrawPixelV(Vector2 position, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawPixelV({position.x * scale.x, position.y * scale.y}, color);
    }

    void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawLine((int)(startPosX * scale.x), (int)(startPosY * scale.y), (int)(endPosX * scale.x),
                (int)(endPosY * scale.y), color);
    }

    void DrawLineV(Vector2 startPos, Vector2 endPos, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawLineV({startPos.x * scale.x, startPos.y * scale.y}, {endPos.x * scale.x, endPos.y * scale.y}, color);
    }

    void DrawLineEx(Vector2 startPos, Vector2 endPos, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawLineEx({startPos.x * scale.x, startPos.y * scale.y}, {endPos.x * scale.x, endPos.y * scale.y}, thickness * scale.y,
                  color);
    }

    void DrawLineStrip(const Vector2* points, int pointCount, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawLineStrip(scaledPoints, pointCount, color);
      delete[] scaledPoints;
    }

    void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawLineBezier({startPos.x * scale.x, startPos.y * scale.y}, {endPos.x * scale.x, endPos.y * scale.y},
                      thickness * scale.y, color);
    }

    void DrawCircle(int centerX, int centerY, float radius, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircle((int)(centerX * scale.x), (int)(centerY * scale.y), radius * scale.y, color);
    }

    void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments,
                          Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircleSector({center.x * scale.x, center.y * scale.y}, radius * scale.y, startAngle, endAngle, segments,
                        color);
    }

    void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle,
                              int segments, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircleSectorLines({center.x * scale.x, center.y * scale.y}, radius * scale.y, startAngle, endAngle,
                              segments, color);
    }

    void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer) {
      Vector2 scale = GetUIScale();
      ::DrawCircleGradient((int)(centerX * scale.x), (int)(centerY * scale.y), radius * scale.y, inner, outer);
    }

    void DrawCircleV(Vector2 center, float radius, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircleV({center.x * scale.x, center.y * scale.y}, radius * scale.y, color);
    }

    void DrawCircleLines(int centerX, int centerY, float radius, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircleLines((int)(centerX * scale.x), (int)(centerY * scale.y), radius * scale.y, color);
    }

    void DrawCircleLinesV(Vector2 center, float radius, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawCircleLinesV({center.x * scale.x, center.y * scale.y}, radius * scale.y, color);
    }

    void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawEllipse((int)(centerX * scale.x), (int)(centerY * scale.y), radiusH * scale.x, radiusV * scale.y, color);
    }

    void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawEllipseLines((int)(centerX * scale.x), (int)(centerY * scale.y), radiusH * scale.x, radiusV * scale.y,
                        color);
    }

    void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle,
                  float endAngle, int segments, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRing({center.x * scale.x, center.y * scale.y}, innerRadius * scale.y, outerRadius * scale.y, startAngle,
                endAngle, segments, color);
    }

    void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle,
                      float endAngle, int segments, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRingLines({center.x * scale.x, center.y * scale.y}, innerRadius * scale.y, outerRadius * scale.y,
                      startAngle, endAngle, segments, color);
    }

    void DrawRectangle(int posX, int posY, int width, int height, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangle((int)(posX * scale.x), (int)(posY * scale.y), (int)(width * scale.x), (int)(height * scale.y),
                      color);
    }

    void DrawRectangleV(Vector2 position, Vector2 size, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleV({position.x * scale.x, position.y * scale.y}, {size.x * scale.x, size.y * scale.y}, color);
    }

    void DrawRectangleRec(Rectangle rectangle, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleRec({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y}, color);
    }

    void DrawRectanglePro(Rectangle rectangle, Vector2 origin, float rotation, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectanglePro({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y},
                        {origin.x * scale.x, origin.y * scale.y}, rotation, color);
    }

    void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleGradientV((int)(posX * scale.x), (int)(posY * scale.y), (int)(width * scale.x),
                              (int)(height * scale.y), top, bottom);
    }

    void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleGradientH((int)(posX * scale.x), (int)(posY * scale.y), (int)(width * scale.x),
                              (int)(height * scale.y), left, right);
    }

    void DrawRectangleGradientEx(Rectangle rectangle, Color topLeft, Color bottomLeft, Color topRight,
                                Color bottomRight) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleGradientEx({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y}, topLeft,
                                bottomLeft, topRight, bottomRight);
    }

    void DrawRectangleLines(int posX, int posY, int width, int height, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleLines((int)(posX * scale.x), (int)(posY * scale.y), (int)(width * scale.x),
                          (int)(height * scale.y), color);
    }

    void DrawRectangleLinesEx(Rectangle rectangle, float lineThick, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleLinesEx({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y},
                            lineThick * scale.y, color);
    }

    void DrawRectangleRounded(Rectangle rectangle, float roundness, int segments, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleRounded({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y}, roundness,
                            segments, color);
    }

    void DrawRectangleRoundedLines(Rectangle rectangle, float roundness, int segments, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleRoundedLines({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y},
                                  roundness, segments, color);
    }

    void DrawRectangleRoundedLinesEx(Rectangle rectangle, float roundness, int segments, float lineThick,
                                    Color color) {
      Vector2 scale = GetUIScale();
      ::DrawRectangleRoundedLinesEx({rectangle.x * scale.x, rectangle.y * scale.y, rectangle.width * scale.x, rectangle.height * scale.y},
                                    roundness, segments, lineThick * scale.y, color);
    }

    void DrawTriangle(Vector2 vertex1, Vector2 vertex2, Vector2 vertex3, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawTriangle({vertex1.x * scale.x, vertex1.y * scale.y}, {vertex2.x * scale.x, vertex2.y * scale.y}, {vertex3.x * scale.x, vertex3.y * scale.y},
                    color);
    }

    void DrawTriangleLines(Vector2 vertex1, Vector2 vertex2, Vector2 vertex3, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawTriangleLines({vertex1.x * scale.x, vertex1.y * scale.y}, {vertex2.x * scale.x, vertex2.y * scale.y}, {vertex3.x * scale.x, vertex3.y * scale.y},
                          color);
    }

    void DrawTriangleFan(const Vector2* points, int pointCount, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawTriangleFan(scaledPoints, pointCount, color);
      delete[] scaledPoints;
    }

    void DrawTriangleStrip(const Vector2* points, int pointCount, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawTriangleStrip(scaledPoints, pointCount, color);
      delete[] scaledPoints;
    }

    void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawPoly({center.x * scale.x, center.y * scale.y}, sides, radius * scale.y, rotation, color);
    }

    void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawPolyLines({center.x * scale.x, center.y * scale.y}, sides, radius * scale.y, rotation, color);
    }

    void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick,
                        Color color) {
      Vector2 scale = GetUIScale();
      ::DrawPolyLinesEx({center.x * scale.x, center.y * scale.y}, sides, radius * scale.y, rotation,
                        lineThick * scale.y, color);
    }

    void DrawSplineLinear(const Vector2* points, int pointCount, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawSplineLinear(scaledPoints, pointCount, thickness * scale.y, color);
      delete[] scaledPoints;
    }

    void DrawSplineBasis(const Vector2* points, int pointCount, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawSplineBasis(scaledPoints, pointCount, thickness * scale.y, color);
      delete[] scaledPoints;
    }

    void DrawSplineCatmullRom(const Vector2* points, int pointCount, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawSplineCatmullRom(scaledPoints, pointCount, thickness * scale.y, color);
      delete[] scaledPoints;
    }

    void DrawSplineBezierQuadratic(const Vector2* points, int pointCount, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawSplineBezierQuadratic(scaledPoints, pointCount, thickness * scale.y, color);
      delete[] scaledPoints;
    }

    void DrawSplineBezierCubic(const Vector2* points, int pointCount, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      Vector2* scaledPoints = new Vector2[pointCount];
      ScalePoints(points, pointCount, scaledPoints, scale);
      ::DrawSplineBezierCubic(scaledPoints, pointCount, thickness * scale.y, color);
      delete[] scaledPoints;
    }

    void DrawSplineSegmentLinear(Vector2 point1, Vector2 point2, float thickness, Color color) {
      Vector2 scale = GetUIScale();
      ::DrawSplineSegmentLinear({point1.x * scale.x, point1.y * scale.y}, {point2.x * scale.x, point2.y * scale.y}, thickness * scale.y, color);
    }

    void DrawSplineSegmentBasis(Vector2 point1, Vector2 point2, Vector2 point3, Vector2 point4, float thickness,
                                Color color) {
      Vector2 scale = GetUIScale();
      ::DrawSplineSegmentBasis({point1.x * scale.x, point1.y * scale.y}, {point2.x * scale.x, point2.y * scale.y},
                              {point3.x * scale.x, point3.y * scale.y}, {point4.x * scale.x, point4.y * scale.y}, thickness * scale.y, color);
    }

    void DrawSplineSegmentCatmullRom(Vector2 point1, Vector2 point2, Vector2 point3, Vector2 point4, float thickness,
                                    Color color) {
      Vector2 scale = GetUIScale();
      ::DrawSplineSegmentCatmullRom({point1.x * scale.x, point1.y * scale.y}, {point2.x * scale.x, point2.y * scale.y},
                                    {point3.x * scale.x, point3.y * scale.y}, {point4.x * scale.x, point4.y * scale.y}, thickness * scale.y,
                                    color);
    }

    void DrawSplineSegmentBezierQuadratic(Vector2 point1, Vector2 control2, Vector2 point3, float thickness,
                                          Color color) {
      Vector2 scale = GetUIScale();
      ::DrawSplineSegmentBezierQuadratic({point1.x * scale.x, point1.y * scale.y}, {control2.x * scale.x, control2.y * scale.y},
                                        {point3.x * scale.x, point3.y * scale.y}, thickness * scale.y, color);
    }

    void DrawSplineSegmentBezierCubic(Vector2 point1, Vector2 control2, Vector2 control3, Vector2 point4, float thickness,
                                      Color color) {
      Vector2 scale = GetUIScale();
      ::DrawSplineSegmentBezierCubic({point1.x * scale.x, point1.y * scale.y}, {control2.x * scale.x, control2.y * scale.y},
                                    {control3.x * scale.x, control3.y * scale.y}, {point4.x * scale.x, point4.y * scale.y}, thickness * scale.y,
                                    color);
    }

    void DrawBackground(Texture2D backgroundTex) {
      ::DrawTexturePro(backgroundTex,
                    Rectangle{0, 0, (float)backgroundTex.width, (float)backgroundTex.height},
                    Rectangle{0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, Vector2{0, 0},
                    0.0f, WHITE);
    }
    }  // namespace Graphic2D
  }  // namespace Fumbo
