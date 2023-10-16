#include <AppKit/AppKit.h>

#include "core.h"

#include <cassert>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#iMetal.hppnclude <MetalKit/MetalKit.hpp>
#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>


#define global_variable static
#define local_persist static
#define internal static

global_variable float global_render_width = 1024;
global_variable float global_render_height = 720;
global_variable bool RUNNING = true;
global_variable u8 *buffer;


#import <AppKit/AppKit.h>

#if 0
struct window
{
    // the data...
    float X, Y;
    float Width, Height;
    uint_32 BackgroundColor;
};

class NSWindow {

    // properties not directly accessible.

    // there is a struct in here!
    struct window
    {
        // the data...
        float X, Y;
        float Width, Height;
        uint_32 BackgroundColor;
    };

    // Initializers
    

    // Constructors
    // Destructors

    @property float Width;

    // Getters and Setters (A.K.A. "accessors")
    - (float) GetWidth {
        return _Width;
    }


    // Because you don't have direct access to the data!!

    /*
    real32 X, Y;
    real32 Width, Height;
    uint32 BackgroundColor;*/
}

struct window
{
    NSRect WindowRect;
    styleMask;
    backingStoreType;
    deferFlag;
    NSColor BackgroundColor;
    NSString Title;
}
#endif

@interface
GameWindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation GameWindowDelegate
-(void)windowWillClose:(NSNotification*)notification {
    [NSApp terminate: nil];
}
@end


int main(int argc, const char * argv[]) {
    NSLog(@"Mooselutions is running!");    

    NSRect WindowRectangle = NSMakeRect(0.0f, 0.0f, 1024.0f, 1024.f);

    NSWindow *Window = [[NSWindow alloc] initWithContentRect: WindowRectangle 
                                                   styleMask: (NSWindowStyleMaskTitled |
                                                               NSWindowStyleMaskClosable)
                                                     backing: NSBackingStoreBuffered 
                                                       defer: NO];
    GameWindowDelegate* WindowDelegate = [[GameWindowDelegate alloc] init];
    [Window setDelegate: WindowDelegate];

    [Window setBackgroundColor: [NSColor redColor]];
    [Window setTitle: @"Mooselutions"];
    [Window makeKeyAndOrderFront: nil];
    
    id<MTLDevice> MetalDevice = MTLCreateSystemDefaultDevice();

    return NSApplicationMain(argc, argv);
}

// View class hierarchy
// NSView (all views inherit from this)
//  |
// MTKView (subclass)

// Views / View Hierarchy
// Hierarchical

// Root View --> A list of instructions for the GPU
//   |
//   Child View
//  |      |
//  CV    CV

// Two types of space of GPU
// 1. Vertex buffers
// 2. Texture memory

// Stuff that needs to get drawn

//  View can contain other views

// Windows 
// NSView
