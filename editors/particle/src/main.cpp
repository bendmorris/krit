#include "imgui.h"
#include "imgui_stdlib.h"
#include "krit/App.h"
#include "krit/Camera.h"
#include "krit/Engine.h"
#include "krit/Options.h"
#include "krit/UpdateContext.h"
#include "krit/editor/Editor.h"
#include "krit/input/InputContext.h"
#include "krit/input/Key.h"
#include "krit/input/Mouse.h"
#include "krit/io/Io.h"
#include "krit/io/ZipIo.h"
#include "krit/sprites/Particles.h"
#include "krit/utils/Log.h"
#include "krit/utils/Signal.h"
#include "nfd.h"
#include <algorithm>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace krit::ped {

enum ParamMode {
    Constant,
    StartEnd,
    StartEndRange,
};

static App *app;

struct AtlasImageInfo {
    std::string atlasPath;
    std::string regionName;
    ImageRegion region;
};

struct ParticleEditor : public DevTool {
    std::vector<std::pair<std::string, float (*)(float)>> ease;
    std::vector<std::pair<std::string, BlendMode>> blend;
    std::shared_ptr<ParticleEffect> effect;
    std::vector<AtlasImageInfo> images;
    std::shared_ptr<TextureAtlas> atlas;
    ParticleSystem *particleSystem;
    bool dirty = false;
    std::string fileName;
    std::string dirName;
    int selectedImage = -1;
    bool showDirtyNewFilePopup = false;
    Color backgroundColor;

    ParticleEditor(ParticleSystem *p) : particleSystem(p) {
        newEffect();
        for (auto &it : interpolationFunctions) {
            ease.push_back(std::make_pair(it.first, it.second));
        }
        ease.push_back(
            std::make_pair<std::string, float (*)(float)>("linear", nullptr));
        std::sort(ease.begin(), ease.end());
        blend.push_back(std::make_pair("alpha", Alpha));
        blend.push_back(std::make_pair("add", Add));
        blend.push_back(std::make_pair("subtract", Subtract));
        blend.push_back(std::make_pair("multiply", Multiply));
        blend.push_back(std::make_pair("screen", BlendScreen));
    }

    void newEffect() {
        effect = std::make_shared<ParticleEffect>();
        effect->name = "effect";
        dirty = false;
    }

    void save() {
        Log::info("saving particle effect: %s", fileName.c_str());
        FILE *f = fopen(fileName.c_str(), "w");
        fprintf(f, "name: %s\nemitters:\n", effect->name.c_str());
        for (auto &emitter : effect->emitters) {
            fprintf(f, "- region: %s\n", emitter.region.c_str());
            switch (emitter.blend) {
                case Add: {
                    fputs("  blend: add\n", f);
                    break;
                }
                case Subtract: {
                    fputs("  blend: subtract\n", f);
                    break;
                }
                case Multiply: {
                    fputs("  blend: multiply\n", f);
                    break;
                }
                case BlendScreen: {
                    fputs("  blend: screen\n", f);
                    break;
                }
                default: {
                }
            }
            if (!emitter.aligned) {
                fprintf(f, "  aligned: false\n");
            }
            fprintf(f, "  count: %i\n", emitter.count);
            if (emitter.zIndex) {
                fprintf(f, "  z: %i\n", emitter.zIndex);
            }
            fprintf(f, "  start: %f\n", emitter.start);
            fprintf(f, "  duration: %f\n", emitter.duration);
            fprintf(f, "  lifetime: [%f,%f]\n", emitter.lifetime.start,
                    emitter.lifetime.end);
            fwriteParam(f, "color", emitter.color);
            fwriteParam(f, "alpha", emitter.alpha);
            if (emitter.scale.start.start != 1 ||
                emitter.scale.start.end != 1 | emitter.scale.end.start != 1 ||
                emitter.scale.end.end != 1) {
                fwriteParam(f, "scale", emitter.scale);
            }
            fwriteParam(f, "distance", emitter.distance);
            fwriteParam(f, "angle", emitter.angle);
            fwriteParam(f, "rotation", emitter.rotation);
            if (emitter.xOffset.start.start || emitter.xOffset.start.end ||
                emitter.xOffset.end.start || emitter.xOffset.end.end) {
                fwriteParam(f, "xOffset", emitter.xOffset);
            }
            if (emitter.yOffset.start.start || emitter.yOffset.start.end ||
                emitter.yOffset.end.start || emitter.yOffset.end.end) {
                fwriteParam(f, "yOffset", emitter.yOffset);
            }
        }
        fclose(f);
        dirty = false;
    }

    template <typename T>
    void fwriteParam(FILE *f, const char *label, ParamRange<T> &param) {
        fprintf(f, "  %s:", label);
        fprintf(f, "\n    value: [");
        fwriteValue(f, param.start.start);
        fputc(',', f);
        fwriteValue(f, param.start.end);
        fputc(',', f);
        fwriteValue(f, param.end.start);
        fputc(',', f);
        fwriteValue(f, param.end.end);
        fputc(']', f);
        fputc('\n', f);
        {
            const char *current;
            for (auto &ease : this->ease) {
                if (param.lerp == ease.second) {
                    current = ease.first.c_str();
                    break;
                }
            }
            if (current && strcmp("linear", current)) {
                fprintf(f, "    ease: %s\n", current);
            }
        }
        fprintf(f, "    relative: %s\n", param.relative ? "true" : "false");
    }

    template <typename T> void fwriteValue(FILE *f, T &value) {}
    template <> void fwriteValue(FILE *f, float &value) {
        fprintf(f, "%f", value);
    }
    template <> void fwriteValue(FILE *f, Color &value) {
        fprintf(f, "[%.3f,%.3f,%.3f]", value.r, value.g, value.b);
    }

    void loadAtlas(const std::string &path) {
        atlas = app->ctx.engine->getAtlas(path);
        images.clear();
        for (auto &it : atlas->regions) {
            images.push_back(
                (AtlasImageInfo){.atlasPath = path,
                                 .regionName = it.first,
                                 .region = atlas->getRegion(it.first)});
        }
        particleSystem->loadAtlas(atlas);
    }

    void setFileName(const std::string &s) {
        fileName = s;
        dirName = std::filesystem::path(s).parent_path();
    }

    void draw(krit::RenderContext &ctx) override {
        if (!atlas) {
            loadAtlas("/home/ben/Dev/march/assets/graphics/particles.atlas");
        }
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "CTRL+N")) {
                    if (dirty) {
                        showDirtyNewFilePopup = true;
                    } else {
                        newEffect();
                    }
                }
                if (ImGui::MenuItem("Open", "CTRL+O")) {
                    // TODO: prompt if dirty
                    nfdchar_t *outPath = nullptr;
                    NFD_OpenDialog("ped",
                                   dirName.empty() ? nullptr : dirName.c_str(),
                                   &outPath);
                    if (outPath) {
                        Log::info("loading particle effect: %s", outPath);
                        effect = ParticleEffect::load(outPath);
                        setFileName(outPath);
                        dirty = false;
                    }
                }
                if (ImGui::MenuItem("Save", "CTRL+S")) {
                    if (fileName.empty()) {
                        nfdchar_t *outPath = nullptr;
                        NFD_SaveDialog(
                            "ped", dirName.empty() ? nullptr : dirName.c_str(),
                            &outPath);
                        if (outPath) {
                            setFileName(outPath);
                            save();
                        }
                    } else {
                        save();
                    }
                }
                if (ImGui::MenuItem("Save As...", "CTRL+A")) {
                    nfdchar_t *outPath = nullptr;
                    NFD_SaveDialog("ped",
                                   dirName.empty() ? nullptr : dirName.c_str(),
                                   &outPath);
                    if (outPath) {
                        setFileName(outPath);
                        save();
                    }
                }
                if (ImGui::MenuItem("Quit", "CTRL+Q")) {
                    // TODO: warn if dirty
                    app->quit();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (showDirtyNewFilePopup) {
            showDirtyNewFilePopup = false;
            ImGui::OpenPopup("Continue without saving?");
        }
        if (ImGui::BeginPopupModal("Continue without saving?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Effect <%s> has unsaved changes. Are you sure "
                        "you want to create a new effect?",
                        effect->name.c_str());
            ImGui::Separator();

            if (ImGui::Button("Yes, lose unsaved changes")) {
                ImGui::CloseCurrentPopup();
                newEffect();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("No, don't create new effect")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_Always, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(360, ctx.window->height() - 64));
        std::string title = "Particle Editor (";
        title += (fileName.empty() ? "untitled" : fileName);
        title += ")";
        if (dirty)
            title += "*";
        if (ImGui::Begin(title.c_str())) {
            if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Background color");
                paramControl(this->backgroundColor);
                ImGui::TreePop();
            }

            // images
            if (ImGui::TreeNodeEx("Images", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (int i = 0; i < images.size(); ++i) {
                    auto &image = images[i];
                    if (ImGui::Selectable(image.regionName.c_str(),
                                          selectedImage == i)) {
                        selectedImage = i;
                    }
                }
                if (ImGui::Button("+Load Atlas")) {
                    nfdchar_t *outPath = NULL;
                    NFD_OpenDialog("atlas", NULL, &outPath);
                    if (outPath) {
                        Log::info("loading atlas: %s", outPath);
                        loadAtlas(outPath);
                    }
                }
                if (selectedImage > -1 && selectedImage < images.size()) {
                    ImGui::SameLine();
                    if (ImGui::Button("+Add Emitter")) {
                        effect->emitters.emplace_back();
                        effect->emitters.back().region =
                            images[selectedImage].regionName;
                        dirty = true;
                    }
                }
                ImGui::TreePop();
            }

            // effect attributes
            if (ImGui::TreeNodeEx("Effect", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::InputText("Name", &effect->name)) {
                    dirty = true;
                }
                ImGui::Text("Duration (s): %.2f", effect->duration());
                ImGui::TreePop();
            }

            // emitters
            if (ImGui::TreeNodeEx("Emitters", ImGuiTreeNodeFlags_DefaultOpen)) {
                ParticleEmitter *dup = nullptr;
                int rem = -1;
                int i = 0;
                for (auto &emitter : effect->emitters) {
                    ImGui::PushID(&emitter);
                    if (ImGui::TreeNodeEx(emitter.region.c_str(),
                                          ImGuiTreeNodeFlags_DefaultOpen)) {

                        if (ImGui::Button("+Duplicate")) {
                            dup = &emitter;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("-Remove")) {
                            rem = i;
                        }
                        ++i;

                        float step = 0.1;

                        const char *current;
                        for (auto &blend : this->blend) {
                            if (emitter.blend == blend.second) {
                                current = blend.first.c_str();
                                break;
                            }
                        }
                        if (ImGui::BeginCombo("blend", current)) {
                            for (auto &blend : this->blend) {
                                bool selected = emitter.blend == blend.second;
                                if (ImGui::Selectable(blend.first.c_str(),
                                                      selected)) {
                                    emitter.blend = blend.second;
                                }
                                if (selected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::Checkbox("aligned", &emitter.aligned);

                        ImGui::Text("start (s)");
                        ImGui::PushID("start");
                        if (ImGui::InputScalar("", ImGuiDataType_Float,
                                               &emitter.start, &step, nullptr,
                                               "%.3f")) {
                            dirty = true;
                        }
                        ImGui::PopID();

                        ImGui::Text("duration (s)");
                        ImGui::PushID("duration");
                        if (ImGui::InputScalar("", ImGuiDataType_Float,
                                               &emitter.duration, &step,
                                               nullptr, "%.3f")) {
                            dirty = true;
                        }
                        ImGui::PopID();

                        ImGui::Text("particle count");
                        ImGui::PushID("count");
                        if (ImGui::InputInt("", &emitter.count)) {
                            dirty = true;
                        }
                        ImGui::PopID();

                        ImGui::Text("Z index");
                        ImGui::PushID("z");
                        if (ImGui::InputInt("", &emitter.zIndex)) {
                            dirty = true;
                        }
                        ImGui::PopID();

                        ImGui::Text("particle lifetime");
                        ImGui::SetNextItemWidth(100);
                        ImGui::PushID("a");
                        if (ImGui::InputScalar("", ImGuiDataType_Float,
                                               &emitter.lifetime.start, &step,
                                               nullptr, "%.3f")) {
                            dirty = true;
                        }
                        ImGui::PopID();
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(100);
                        ImGui::PushID("b");
                        if (ImGui::InputScalar("", ImGuiDataType_Float,
                                               &emitter.lifetime.end, &step,
                                               nullptr, "%.3f")) {
                            dirty = true;
                        }
                        ImGui::PopID();

                        paramRange("color", emitter.color);
                        paramRange("alpha", emitter.alpha);
                        paramRange("scale", emitter.scale);
                        paramRange("distance", emitter.distance);
                        paramRange("angle", emitter.angle);
                        paramRange("rotation", emitter.rotation);
                        paramRange("xOffset", emitter.xOffset);
                        paramRange("yOffset", emitter.yOffset);

                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                if (dup) {
                    ParticleEmitter e;
                    e.color = dup->color;
                    e.alpha = dup->alpha;
                    e.scale = dup->scale;
                    e.distance = dup->distance;
                    e.angle = dup->angle;
                    e.rotation = dup->rotation;
                    e.xOffset = dup->xOffset;
                    e.yOffset = dup->yOffset;
                    e.lifetime = dup->lifetime;
                    printf("%s -> %s\n", e.region.c_str(), dup->region.c_str());
                    e.region = dup->region;
                    printf("%s -> %s\n", e.region.c_str(), dup->region.c_str());
                    e.blend = dup->blend;
                    e.aligned = dup->aligned;
                    e.count = dup->count;
                    e.start = dup->start;
                    e.duration = dup->duration;
                    e.zIndex = dup->zIndex;
                    effect->emitters.emplace_back(std::move(e));
                    dirty = true;
                }
                if (rem > -1) {
                    effect->emitters.erase(effect->emitters.begin() + rem);
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    template <typename T>
    void paramRange(const std::string &label, ParamRange<T> &value,
                    float min = 0) {
        ImGui::PushID(&value);

        if (ImGui::TreeNode(label.c_str())) {
            ImGui::PushID("a");
            paramControl<T>(value.start.start);
            ImGui::PopID();

            ImGui::PushID("b");
            paramControl<T>(value.start.end, true);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::Text("start");

            ImGui::PushID("c");
            paramControl<T>(value.end.start);
            ImGui::PopID();

            ImGui::PushID("d");
            paramControl<T>(value.end.end, true);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::Text("end");

            relativeAndEase(value);

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    template <typename T> void paramControl(T &value, bool second = false) {}
    template <> void paramControl(float &value, bool second) {
        ImGui::PushID(&value);
        float step = 0.1;
        ImGui::SetNextItemWidth(100);
        if (second) {
            ImGui::SameLine();
        }
        if (ImGui::InputScalar("", ImGuiDataType_Float, &value, &step, nullptr,
                               "%.3f")) {
            dirty = true;
        }
        ImGui::PopID();
    }

    template <> void paramControl(Color &color, bool second) {
        float c[3]{
            color.r,
            color.g,
            color.b,
        };
        if (ImGui::ColorEdit3("", c)) {
            dirty = true;
            color.setTo(c[0], c[1], c[2]);
        }
    }

    template <typename T> void relativeAndEase(ParamRange<T> &value) {
        ImGui::Checkbox("relative", &value.relative);

        const char *current;
        for (auto &ease : this->ease) {
            if (value.lerp == ease.second) {
                current = ease.first.c_str();
                break;
            }
        }
        if (ImGui::BeginCombo("ease", current)) {
            for (auto &ease : this->ease) {
                bool selected = value.lerp == ease.second;
                if (ImGui::Selectable(ease.first.c_str(), selected)) {
                    value.lerp = ease.second;
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
};

void appMain() {
    KritOptions options;
    options.setTitle("Particle Editor").setSize(1280, 720).setFrameRate(60);
    app = new App(options);
    Engine &engine = app->engine;
    std::unique_ptr<Editor> editor;
    std::unique_ptr<ParticleSystem> p =
        std::unique_ptr<ParticleSystem>(new ParticleSystem());
    ParticleEditor *ped = new ParticleEditor(p.get());

    engine.onBegin = [&](UpdateContext *ctx) {
        ctx->engine->input.bindMouse(MouseLeft, 1);
        ctx->engine->input.bindMouse(MouseRight, 2);
        editor = std::unique_ptr<Editor>(new Editor());
        editor->addDevTool(ped);
    };

    engine.onUpdate = [&](UpdateContext *ctx) {
        p->update(*ctx);
        for (auto &it : ctx->engine->input.events) {
            if ((it.action == 1 && it.state && !it.prevState) ||
                (it.action == 2)) {
                p->registerEffect(ped->effect);
                Point mousePos = ctx->engine->input.mouse.mousePos;
                ctx->engine->cameras[0].untransformPoint(mousePos);
                p->emit(ped->effect->name, mousePos);
            }
        }
    };

    engine.cameras.reserve(2);
    {
        auto &mainCamera = engine.addCamera();
        mainCamera.setLogicalSize(3840, 2160).keepHeight(2160, 2160);
        mainCamera.render = [&](RenderContext *ctx) {
            if (ped->backgroundColor.r + ped->backgroundColor.g +
                ped->backgroundColor.b) {
                ctx->drawRect(0, 0, 3840, 2160, ped->backgroundColor, 1);
            }
            p->render(*ctx);
        };
    }
    {
        auto &uiCamera = engine.addCamera();
        uiCamera.render = [&](RenderContext *ctx) {
            editor->update(*ctx);
            editor->render(*ctx);
        };
    }

    app->run();
}
}

int main(int argc, char *argv[]) {
    krit::ped::appMain();
    return 0;
}

#if defined(_WIN32)

#include <windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,
            INT nCmdShow) {
    main(0, nullptr);
    return 0;
}

#endif
