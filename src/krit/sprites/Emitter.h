#ifndef KRIT_SPRITES_EMITTER
#define KRIT_SPRITES_EMITTER

#include "krit/asset/AssetContext.h"
#include "krit/render/BlendMode.h"
#include "krit/render/ImageRegion.h"
#include "krit/sprites/Image.h"
#include "krit/sprites/UserSprite.h"
#include "krit/utils/Color.h"
#include "krit/Sprite.h"
#include "krit/Math.h"
#include "krit/Utils.h"
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>

namespace krit {

typedef double (*LerpFunction)(double);

struct ParticleType {
    int id;
    Image image;

    Range<Color> colorStart;
    Range<Color> colorEnd;
    LerpFunction colorLerp = nullptr;
    Range<double> alphaStart;
    Range<double> alphaEnd;
    LerpFunction alphaLerp = nullptr;
    Range<double> scaleStart;
    Range<double> scaleEnd;
    LerpFunction scaleLerp = nullptr;
    Range<double> distanceStart;
    Range<double> distanceEnd;
    LerpFunction distanceLerp = nullptr;
    Range<double> rotationStart;
    Range<double> rotationEnd;
    LerpFunction rotationLerp = nullptr;
    Range<double> angle;

    Range<double> duration;
    Point offset;
    double trail = 0;

    ParticleType(int id, ImageRegion region, BlendMode blend = Alpha);

    ParticleType &setColor(Color color) {
        return this->setColor(color, color, color, color);
    }
    ParticleType &setColor(Color start, Color end) {
        return this->setColor(start, start, end, end);
    }
    ParticleType &setColor(Color start1, Color start2, Color end1, Color end2) {
        this->colorStart.setTo(start1, start2);
        this->colorEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setColorLerp(LerpFunction f) {
        this->colorLerp = f;
        return *this;
    }

    ParticleType &setScale(double start, double end) {
        return this->setScale(start, start, end, end);
    }
    ParticleType &setScale(double start1, double start2, double end1, double end2) {
        this->scaleStart.setTo(start1, start2);
        this->scaleEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setScaleLerp(LerpFunction f) {
        this->scaleLerp = f;
        return *this;
    }

    ParticleType &setDistance(double start, double end) {
        return this->setDistance(start, start, end, end);
    }
    ParticleType &setDistance(double start1, double start2, double end1, double end2) {
        this->distanceStart.setTo(start1, start2);
        this->distanceEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setDistanceLerp(LerpFunction f) {
        this->distanceLerp = f;
        return *this;
    }

    ParticleType &setRotation(double start, double end) {
        return this->setRotation(start, start, end, end);
    }
    ParticleType &setRotation(double start1, double start2, double end1, double end2) {
        this->rotationStart.setTo(start1, start2);
        this->rotationEnd.setTo(end1, end2);
        return *this;
    }
    ParticleType &setRotationLerp(LerpFunction f) {
        this->rotationLerp = f;
        return *this;
    }

    ParticleType &setDuration(double duration) {
        return this->setDuration(duration, duration);
    }
    ParticleType &setDuration(double start, double end) {
        this->duration.setTo(start, end);
        return *this;
    }

    ParticleType &setAngle(double angle) {
        return this->setAngle(angle, angle);
    }
    ParticleType &setAngle(double start, double end) {
        this->angle.setTo(start, end);
        return *this;
    }

    ParticleType &setBlend(BlendMode blend) {
        this->image.blendMode = blend;
        return *this;
    }

    ParticleType &setOffset(double x, double y) {
        this->offset.setTo(x, y);
        return *this;
    }

    ParticleType &setTrail(double trail) {
        this->trail = trail;
        return *this;
    }
};

struct Particle {
    ParticleType *type;
    double decay = 0;
    double duration;
    Point start;
    Point origin;
    Point move;
    Range<Color> color;
    Range<double> alpha;
    Range<double> scale;
    Range<double> rotation;
    bool isTrail = false;
    double angle = 0;

    Particle(ParticleType *type): type(type) {}
};

struct ParticleEffect {
    std::vector<std::pair<int, Range<int>>> types;

    ParticleEffect &addType(int type, int min, int max = -1) {
        if (max == -1) {
            max = min;
        }
        types.push_back(std::make_pair(type, Range<int>(min, max)));
        return *this;
    }
};

struct ParticleSystem {
    std::vector<ParticleEffect> effects;
    std::vector<ParticleType> types;

    ParticleType &defineType(ImageRegion region, BlendMode blend = Alpha) {
        types.emplace_back(types.size(), region, blend);
        return types.back();
    }

    ParticleEffect &defineEffect() {
        effects.emplace_back();
        return effects.back();
    }
};

struct Emitter: public VisibleSprite {
    ParticleSystem &system;

    Emitter(ParticleSystem &system): system(system) {}

    std::size_t particleCount() { return _particles.size(); }

    void emit(int effectType);
    void emit(ParticleType &type, int count);

    Dimensions getSize() override { return Dimensions(0, 0); }
    void resize(double w, double h) override {}

    void update(UpdateContext &ctx) override;
    void render(RenderContext &ctx) override;

    private:
        std::vector<Particle> _particles;
};

}

#endif
