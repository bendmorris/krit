#ifndef KRIT_SPRITES_EMITTER
#define KRIT_SPRITES_EMITTER

#include "krit/Math.h"
#include "krit/Sprite.h"
#include "krit/Utils.h"
#include "krit/asset/TextureAtlas.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/sprites/Image.h"
#include "krit/sprites/UserSprite.h"
#include "krit/utils/Color.h"
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>

namespace krit {

typedef float (*LerpFunction)(float);

struct ParticleType {
    int index;
    Image image;

    Range<Color> colorStart;
    Range<Color> colorEnd;
    LerpFunction colorLerp = nullptr;
    Range<float> scaleStart;
    Range<float> scaleEnd;
    LerpFunction scaleLerp = nullptr;
    Range<float> distanceStart;
    Range<float> distanceEnd;
    LerpFunction distanceLerp = nullptr;
    Range<float> rotationStart;
    Range<float> rotationEnd;
    LerpFunction rotationLerp = nullptr;
    Range<float> angle;

    Range<float> duration;
    Point offset;
    float trail = 0;

    ParticleType(int id, ImageRegion region, BlendMode blend = Alpha);

    ParticleType &setColor(const Color &color) {
        return this->setColor(color, color, color, color);
    }
    ParticleType &setColor(const Color &start, const Color &end) {
        return this->setColor(start, start, end, end);
    }
    ParticleType &setColor(const Color &start1, const Color &start2,
                           const Color &end1, const Color &end2) {
        this->colorStart.setTo(start1, start2);
        this->colorEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setColorLerp(LerpFunction f) {
        this->colorLerp = f;
        return *this;
    }

    ParticleType &setScale(float start, float end) {
        return this->setScale(start, start, end, end);
    }
    ParticleType &setScale(float start1, float start2, float end1, float end2) {
        this->scaleStart.setTo(start1, start2);
        this->scaleEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setScaleLerp(LerpFunction f) {
        this->scaleLerp = f;
        return *this;
    }

    ParticleType &setDistance(float start, float end) {
        return this->setDistance(start, start, end, end);
    }
    ParticleType &setDistance(float start1, float start2, float end1,
                              float end2) {
        this->distanceStart.setTo(start1, start2);
        this->distanceEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setDistanceLerp(LerpFunction f) {
        this->distanceLerp = f;
        return *this;
    }

    ParticleType &setRotation(float start, float end) {
        return this->setRotation(start, start, end, end);
    }
    ParticleType &setRotation(float start1, float start2, float end1,
                              float end2) {
        this->rotationStart.setTo(start1, start2);
        this->rotationEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setRotationLerp(LerpFunction f) {
        this->rotationLerp = f;
        return *this;
    }

    ParticleType &setDuration(float duration) {
        return this->setDuration(duration, duration);
    }
    ParticleType &setDuration(float start, float end) {
        this->duration.setTo(start, end);
        return *this;
    }

    ParticleType &setAngle(float angle) { return this->setAngle(angle, angle); }
    ParticleType &setAngle(float start, float end) {
        this->angle.setTo(start, end);
        return *this;
    }

    ParticleType &setBlend(BlendMode blend) {
        this->image.blendMode = blend;
        return *this;
    }

    ParticleType &setOffset(float x, float y) {
        this->offset.setTo(x, y);
        return *this;
    }

    ParticleType &setTrail(float trail) {
        this->trail = trail;
        return *this;
    }

    ParticleType &setZ(int z) {
        this->image.zIndex = z;
        return *this;
    }
};

struct Particle {
    int typeIndex;
    float decay = 0;
    float duration;
    Point start;
    Point origin;
    Point move;
    Range<Color> color;
    Range<float> alpha;
    Range<float> scale;
    Range<float> rotation;
    bool isTrail = false;
    float angle = 0;

    Particle(int index) : typeIndex(index) {}
};

struct ParticleSystem {
    std::shared_ptr<TextureAtlas> atlas;
    std::vector<ParticleType> types;

    ParticleSystem() {}
    ParticleSystem(std::shared_ptr<TextureAtlas> atlas) : atlas(atlas) {}

    ParticleType &defineType(const std::string &name,
                             const std::string &regionName,
                             BlendMode blend = Alpha) {
        _typeMap[name] = types.size();
        types.emplace_back(types.size(), atlas->getRegion(regionName), blend);
        return types.back();
    }

    ParticleType &get(const std::string &name) { return types[_typeMap[name]]; }

private:
    std::unordered_map<std::string, int> _typeMap;
};

struct Emitter : public VisibleSprite {
    ParticleSystem &system;

    Emitter(ParticleSystem &system) : system(system) {}

    std::size_t particleCount() { return _particles.size(); }

    void emit(const std::string &, int count);
    void emit(ParticleType &type, int count);

    Dimensions getSize() override { return Dimensions(0, 0); }
    void resize(float w, float h) override {}

    void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

private:
    std::vector<Particle> _particles;
};

}

#endif
