
#include <stdio.h>
#include <AppKit/AppKit.h>

typedef global_variable static

global_variable float global_render_width = 1024;
static float global_render_height = 720;
static bool RUNNING = true;

@interface MainWindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation MainWindowDelegate
-(void)WindowWillClose:(id)sender {
    RUNNING = false;
}
@end

int main(int argc, char** argv) {

    MainWindowDelegate* main_window_delegate = [[MainWindowDelegate alloc] init];

    NSRect screen_rect = [[NSScreen main_screen] frame];

    NSRect window_rect = NSMakeRect((screen_rect.size.width - global_render_width) * .5, 
                                    (screen_rect.size.height - global_render_height) * .5, 
                                    global_render_width, 
                                    global_render_height);

    NSWindow* window = [[NSWindow alloc] initWithContentRect:initial_frame]
                                        styleMask: NSWindowStyleMaskTitled |
                                                   NSWindowStyleMaskClosable |
                                                   NSWindowStyleMaskMiniaturizable |
                                                   NSWindowStyleMaskResizable
                                        backing:NSBackingStoreBuffered
                                        defer:NO];

    [NSApplication sharedApplication];
    
    printf("You are great!\n");

    while(RUNNING) {

        NSEvent* event;
        do {
            event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                       untilDate: nil
                                          inMode: NSDefaultRunLoopMode
                                         dequeue: YES];
            switch ([event type]) {
                default:
                [NSApp sendEvent: event];
            }
        } while (event != nil);
    }

    printf("Finished running.");
    return 0;
}