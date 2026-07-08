#ifndef TRACK_H
#define TRACK_H

#include <vector>
#include <string>
#include <GL/glew.h>

struct PathPoint {
    float x;
    float z;
    float angle;
};

class Track {
public:
    Track();
    ~Track();

    bool Load(const std::string& filename);
    void CreateDefaultTrack();
    void InitGraphics();
    void Render(float playerProgress, float playerSpeed);
    void Cleanup();

    bool CheckCollision(float posX, float posZ, float carWidth, float carLength);

    float GetTrackLength() const { return trackLength; }
    PathPoint GetPathPoint(float distance) const;
    PathPoint GetLocalSample(float distAhead, float playerProgress) const;
    float GetTrackOffsetX(float progress) const;
    float GetTrackHeading(float progress) const;

    // Modern OpenGL rendering
    void CreateTrackGeometry();
    void RenderModern(float playerProgress, float playerSpeed);
    void Draw3D(float playerProgress, float playerSpeed, GLuint shaderProgram);

private:
    struct TrackSegment {
        float curve;
        float length;
    };

    std::vector<TrackSegment> segments;
    float trackWidth;
    float trackLength;

    GLuint asphaltTex;
    GLuint grassTex;
    GLuint fenceTex;

    // Modern OpenGL resources
    GLuint trackVAO;
    GLuint trackVBO;
    GLuint trackEBO;
    GLuint trackIndexCount;

    float WrapDistance(float distance) const;
    void DrawGround(float playerProgress, GLuint shaderProgram);
    void DrawRoadStrip(float playerProgress, GLuint shaderProgram);
    void DrawTrackEdgeStrip(float playerProgress, bool leftSide, bool isFence, GLuint shaderProgram);
    void DrawGrandstands(float playerProgress, GLuint shaderProgram);
};

#endif