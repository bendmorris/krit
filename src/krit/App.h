#ifndef KRIT_APP
#define KRIT_APP

#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/Backend.h"
#include "krit/Engine.h"
#include "krit/Options.h"
#include "krit/UpdateContext.h"

using namespace std;
using namespace krit;

namespace krit {

const int MAX_FRAMES = 5;
const int FPS = 60;

struct App {
    string title;
    Engine engine;
    IntDimensions window;
    // opaque, backend-specific contents
    Backend backend;
    Renderer renderer;

    App(KritOptions &options);

    /**
     * Run the app; will continue until
     */
    void run();

    /**
     * Ends the run() loop.
     */
    void quit() {
        this->running = false;
    }

    void setFullScreen(bool full);
    bool isFullScreen() { return this->full; }

    private:
        bool running = false;
        bool full = false;

        void flushRender(RenderContext &render) {
            this->renderer.flushBatch(render);
            this->renderer.flushFrame(render);
        }

        #ifndef SINGLE_THREAD
        static int renderLoop(void*);
        #endif

        void handleEvents(UpdateContext&);
};

}

#endif
