#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H
#include "types.h"


// physical location of a pixel on the device
struct PixelLocation{
    uint16_t x;
    uint16_t y;
};



Colour lerp(Colour a, Colour b, uint16_t alpha)
{
    Colour ret;
    ret.r = ((a.r * alpha) + (b.r * (1<<16 - alpha))) >> 16;
    ret.g = ((a.g * alpha) + (b.g * (1<<16 - alpha))) >> 16;
    ret.b = ((a.b * alpha) + (b.b * (1<<16 - alpha))) >> 16;
    return ret;
}





// describe a frame buffer and how to raster over it.
template <size_t NUMPIXELS, class IMPL>
class Display{
    public:

    // let users know how many pixels to iterate over
    static const size_t NumPixels = NUMPIXELS;

    // define a buffer with the correct size
    Colour buffer[NUMPIXELS];

    // require implementations to say where the pixels are.
    PixelLocation CoordAtIndex(size_t raster_index){
        return IMPL::CoordAtIndexImplr_index;
    }
};


// implement a display type that is just a bar of pixels
class LedBarDisplay : public Display<7, LedBarDisplay>
{
    public:
    static PixelLocation CoordAtIndexImpl(size_t raster_index){
        PixelLocation loc;
        loc.x = raster_index;
        loc.y = 0;
        return loc;
    }
};




// virtual class to capture functionality of an animation
template <class DISPLAY_TYPE, class IMPL>
class Animation
{
    typedef FrameBuffer DISPLAY_TYPE;

private:
    // XXX should probably specify the max number FrameBuffers

    // frame_buffers are provided by controller so animations can fade from the colour of the previous animation
    FrameBuffer* frame_buffers;

public:

    // deduce the colour of a single pixel
    virtual Colour render(Position position, unsigned long time_ms);

    void set_buffers(FrameBuffer* buffers)
    {
        frame_buffers = buffers;
    }
    // let the controller know when it's done, and what to do after
    unsigned long max_time_ms() {return IMPL::max_time_ms();}
    const bool looping() = 0;

};




template <class DISPLAY_TYPE>
class fadeToColour : public Animation<DISPLAY_TYPE, fadeToColour>
{
public:
    typedef FrameBuffer DISPLAY_TYPE;
private:
    unsigned long duration_ms;
    FrameBuffer* target;
public:

    fadeToColour(unsigned long duration_ms, FrameBuffer* target):
        duration_ms(duration_ms),
        target(target),
    {
        //nothing to do
    }

    const unsigned long max_time_ms()
    {
        return duration_ms;
    }

    Colour render(Position position, unsigned long time_ms)
    {
        uint16_t alpha = ((1<<16) *  time_ms) / duration_ms;
        return lerp(frame_buffers[0][position], *target[position], alpha);
    }
};


template <class DISPLAY_TYPE>
class AnimationController{
public:
    typedef FrameBuffer DISPLAY_TYPE;
    using callback_t = void(*)(void*);

private:
    // XXX it never actaully passes the framebuffers and pallets to the animation.
    FrameBuffer display;
    FrameBuffer frame_buffers[1];

    Animation* current_animation;

    // the cpu time when the current animation started
    unsigned long datum_time;
    // the cpu time when the last frame was rendered
    unsigned long latest_time;


    // XXX needs a hardware update callback function pointer


    callback_t animation_complete_callback;
    void* animation_complete_callback_context;

public:

    void set_animation_complete_callback(cb, ctx)
    {
        animation_complete_callback = cb;
        animation_complete_callback_context = ctx;
    }


    // convert the cou time into an animation time
    unsigned_long get_animation_time(unsigned long cpu_time_ms)
    {
        return datum_time - cpu_time_ms;
    }

    // write down the current display state and switch animations
    void change_animation(Animation* new_animation)
    {
        // hand the current displayed frame to the next animation
        display = frame_buffers[0];
        // set the time-zero for the new animation
        datum_time = latest_time;
        // switch to the newly provided animation
        current_animation = new_animation;
        current_animation->set_buffers(frame_buffers);
        current_animation = new_animation;
    }

    // ask the current animation how to render this pixel
    Colour render(Position position, unsigned long animation_time_ms)
    {
        // check if we need to load a new animation
        if(animation_time_ms > current_animation->max_time_ms()){
            if(animation_complete_callback){
                animation_complete_callback(animation_complete_callback_context);
            }
        }
        //compute the pixel value
        return current_animation->render(position, animation_time_ms);
    }

    // iterate over the display
    void raster(unsigned long cpu_time_ms)
    {
        // remember the time the last frame was drawn
        latest_time = time_ms;

        // figure out how long this animation has been cooking
        animation_time_ms = get_animation_time(cpu_time_ms);
        //draw all the pixels
        for(size_t pixel = 0; pixel<DISPLAY_TYPE::NumPixels; pixels++)
        {
            PixelLocation pxloc = DISPLAY_TYPE::CoordAtIndex(size_t pixel);
            Colour pxcol = render(pxloc, animation_time_ms);
            display.pixels[pixel] = pxcol;
            // XXX run a callback to update the hardware
        }
    }
};



// A rough example for how to switch between different animations on the controller
template <class DISPLAY_TYPE>
class AnimationSequencer{

    using Anim = Animation<DISPLAY_TYPE>
    using AnimController = AnimationController<DISPLAY_TYPE>

    // we want access to the controller
    AnimController* controller;

    // remember which animation we're up to

    size_t n_anims =3;
    Anim animations[n_anims] = {
        boot_animation(),
        fadeToColour(1000,0),
        fadeToColour(1000,0),
        fadeToColour(1000,0),
    };
    size_t current_anim = 0;


    // misc extra business logic so you can change animations in response to user input
    void button_input(){
        // mess with current anim,
        // make different decisions about what's next.
        // talk directly to the controller etc
    }

    Anim* next_animation()
    {
        // example state machine where we just loop
        Anim* next_anim = animations[current_anim];
        if(current_anim < n_anims){
            current_anim++;
        }
        else
        {
            current_anim = 0;
        }
        return next_anim;
    };

    // this is a callback, called by render() once the previous animation is over
    static void animation_finished(void* ctx)
    {
        // unpack the context void pointer into a usabel type
        AnimationSequencer* self = ctx;

        // update the animation controller with a new animation
        self->controller.change_animation(self->next_animation());

        // update which callback it should call
        // this part allows you to switch over to a whole other sequencer
        self->controller.set_animation_complete_callback(AnimationSequencer::animation_finished, self);
    };

};
#endif // TYPES_H
