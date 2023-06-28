#include <stdio.h> 
#include <AppKit/AppKit.h>
#import <IOKit/hid/IOHIDLib.h>
#import <AudioToolbox/AudioToolbox.h>
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <math.h>
#include "osx_main.h"
#include <sys/stat.h>


#define global_variable static
global_variable bool Running = true;

// TODO: Destination bounds checking
static void
catStrings(size_t source_a_count, char *source_a,
           size_t source_b_count, char *source_b,
           size_t dest_count, char *dest)
{
    
    for(uint32 index = 0;
        index < source_a_count;
        ++index)
    {
        *dest++ = *source_a++;
    }

    for(uint32 index = 0;
        index < source_b_count;
        ++index)
    {
        *dest++ = *source_b++;
    }

    *dest++ = 0;
}

static uint32
stringLength(char *string)
{
    uint32 count = 0;
    while(*string++)
    {
        ++count;
    }
    return(count);
}

static void
macBuildAppPathFilename(OSX_AppPath *path, char* filename, uint32 dest_count, char* dest)
{
    size_t path_file_name_size = (size_t)(path->one_past_last_app_file_name_slash - path->filename);
    catStrings(path_file_name_size, path->filename,
               stringLength(filename), filename,
               dest_count, dest);
}

// NOTE: (Ted)  Not sure how this performs when the app directory is a symbolic link
static void
macBuildAppFilePath(OSX_AppPath *path)
{
    uint32 buffer_size = sizeof(path->filename);
    if (_NSGetExecutablePath(path->filename, &buffer_size) == 0)
    {
        for (char *scan = path->filename;
             *scan; scan++)
        {
            if (*scan == '/')
            {
                path->one_past_last_app_file_name_slash = scan + 1;
            }
        }
    }
}

//###########  DEBUG FUNCTIONS
#if HANDMADE_INTERNAL
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (memory)
    {
        free(memory);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    // TODO: (Jon) get this struct
    Debug_Read_File_Result result = {};

    OSX_AppPath path = {};
    macBuildAppFilePath(&path);

    // NOTE: (Ted)  This is the file location in the bundle.
    char bundle_filename[MAC_MAX_FILENAME_SIZE];
    char local_filename[MAC_MAX_FILENAME_SIZE];

    /*
    // TODO: (Ted)  Put all of this into our own string functions.
    printf(local_filename, "Contents/Resources/%s", filename);
    */

    // Contents/Resources/test_background.bmp
    macBuildAppPathFilename(&path, local_filename,
                            sizeof(bundle_filename), bundle_filename);

    // TODO: (Ted)  Actually load the bitmap!!!
    FILE *file_handle = fopen(bundle_filename, "r+");

    if (file_handle != NULL)
    {
        fseek(file_handle, 0, SEEK_END);
        uint64 file_size = (uint64)ftell(file_handle);

        if (file_size)
        {
        	rewind(file_handle);
            result.contents = malloc(file_size);

            if (result.contents)
            {
                uint64 bytes_read = fread(result.contents, 1, file_size, file_handle);
                if (bytes_read == file_size)
                {
                    // Successfully read the file.
                    result.contents_size = (uint32)file_size;

                } else
                {
                    // TODO: Clean this up. 
                    //DEBUGPlatformFreeFileMemory(result.contents);
                    result.contents_size = 0;
                }
            } else
            {
                // TODO: Log this.
            }
        } else
        {
            // TODO: Log this.
        }
    } else 
    {
        // TODO: Log this.
    }

    return (result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 result = false;

    // TODO: Ted    Consider cleaning this up / compressing it?
    OSX_AppPath path = {};
    macBuildAppFilePath(&path);

    char bundle_filename[MAC_MAX_FILENAME_SIZE];
    char local_filename[MAC_MAX_FILENAME_SIZE];

    //sprintf(local_file_name, "Contents/%s", filename);

    macBuildAppPathFilename(&path, local_filename,
                            sizeof(bundle_filename), bundle_filename);

    FILE* file_handle = fopen(bundle_filename, "w");

    if (file_handle)
    {
        uint64 bytes_written = fwrite(memory, 1, memory_size, file_handle);
        if (bytes_written == memory_size)
        {
            result = true;
        } else
        {
            // TODO: Log this.
        }
    } else
    {
        // TODO: Log this
    }

    return result;
}
#endif

// ########### CONTROLLER SECTION

static
void controllerInput(void* context, IOReturn result, 
                     void* sender, IOHIDValueRef value)
{

    if(result != kIOReturnSuccess) {
        return;
    }

    Mac_Game_Controller* mac_game_controller = (Mac_Game_Controller*)context;
    
    IOHIDElementRef element = IOHIDValueGetElement(value);    
    uint32 usage_page = IOHIDElementGetUsagePage(element);
    uint32 usage = IOHIDElementGetUsage(element);

    //Buttons
    if(usage_page == kHIDPage_Button) {
        // TODO: (ted)  Use our own Boolean type here?
        BOOL button_state = (BOOL)IOHIDValueGetIntegerValue(value);

        if (usage == mac_game_controller->osx_usage_id->bottom_action_usage_ID)
        {
            mac_game_controller->game_controller->action_down.ended_down = button_state;
        }
        /*
        else if (usage == osx_game_controller->left_button_usage_ID)
        {
            osx_game_controller->left_button_state = button_state;
        }
        else if (usage == osx_game_controller->top_button_usage_ID)
        {
            osx_game_controller->top_button_state = button_state;
        }
        else if (usage == osx_game_controller->right_button_usage_ID)
        {
            osx_game_controller->right_button_state = button_state;
        }
        else if (usage == osx_game_controller->left_shoulder_button_usage_ID)
        {
            osx_game_controller->left_shoulder_button_state = button_state;
        }
        else if (usage == osx_game_controller->right_shoulder_button_usage_ID)
        {
            osx_game_controller->right_shoulder_button_state = button_state;
        }
        */
    }
    else if (usage_page == kHIDPage_GenericDesktop)
    {
        /*
        double_t analog = IOHIDValueGetScaledValue(value, kIOHIDValueScaleTypeCalibrated);

        // NOTE: (ted)  It seems like slamming the stick left gives me a value of zero 
        //              and slamming it all the way right gives a value of 255. 
        //
        //              I would gather this is being mapped to an eight bit unsigned integer

        //              Max Y up is zero. Max Y down is 255. Not moving Y is 128.
        if (usage == osx_game_controller->left_thumb_x_usage_ID) {
            osx_game_controller->left_thumbstick_X = (real32)analog;
        }

        if (usage == osx_game_controller->left_thumb_Y_usageID) {
            osx_game_controller->left_thumbstick_Y = (real32)analog;
        }

        if(usage == kHIDUsage_GD_Hatswitch) { 
            int d_pad_state = (int)IOHIDValueGetIntegerValue(value);
            int32 d_pad_x = 0;
            int32 d_pad_y = 0;

            switch(d_pad_state) {
                case 0: d_pad_x = 0;  d_pad_y = 1; break;
                case 1: d_pad_x = 1;  d_pad_y = 1; break;
                case 2: d_pad_x = 1;  d_pad_y = 0; break;
                case 3: d_pad_x = 1;  d_pad_y = -1; break;
                case 4: d_pad_x = 0;  d_pad_y = -1; break;
                case 5: d_pad_x = -1; d_pad_y = -1; break;
                case 6: d_pad_x = -1; d_pad_y = 0; break;
                case 7: d_pad_x = -1; d_pad_y = 1; break;
                default: d_pad_x = 0; d_pad_y = 0; break;
            }

            osx_game_controller->d_pad_x = d_pad_x;
            osx_game_controller->d_pad_y = d_pad_y;
        }
        */
    }
}

static 
void controllerConnected(void* context, IOReturn result, 
                         void* sender, IOHIDDeviceRef device)
{
    if(result != kIOReturnSuccess) {
        return;
    }

    NSUInteger vendor_ID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, 
                                                                       CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
    NSUInteger product_ID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, 
                                                                        CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];

    Mac_Game_Controller* mac_game_controller = (Mac_Game_Controller*)context;

    if(vendor_ID == 0x054C && product_ID == 0x5C4) {
        NSLog(@"Sony Dualshock 4 detected.");
        mac_game_controller->osx_usage_id->bottom_action_usage_ID = 0x02;
        mac_game_controller->osx_usage_id->left_action_usage_ID = 0x01;
        mac_game_controller->osx_usage_id->top_action_usage_ID = 0x04;
        mac_game_controller->osx_usage_id->right_action_usage_ID = 0x03;
        mac_game_controller->osx_usage_id->left_shoulder_button_usage_ID = 0x05;
        mac_game_controller->osx_usage_id->right_shoulder_button_usage_ID = 0x06;

        mac_game_controller->osx_usage_id->left_thumb_x_usage_ID = kHIDUsage_GD_X;
        mac_game_controller->osx_usage_id->left_thumb_y_usage_ID = kHIDUsage_GD_Y;
    }

    mac_game_controller->game_controller->left_stick_average_x = 128.0f;
    mac_game_controller->game_controller->left_stick_average_y = 128.0f;

    IOHIDDeviceRegisterInputValueCallback(device, controllerInput, (void*)mac_game_controller);  

    IOHIDDeviceSetInputValueMatchingMultiple(device, (__bridge CFArrayRef)@[
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_GenericDesktop)},
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_Button)},
    ]);
}

static void
osxSetupGameController(Mac_Game_Controller* mac_game_controller)
{
    IOHIDManagerRef HIDManager = IOHIDManagerCreate(kCFAllocatorDefault, 0);

    // TODO: (ted)  Actually handle errors better
    if (IOHIDManagerOpen(HIDManager, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
        NSLog(@"Error Initializing OSX Handmade Controllers");
        return;
    }
    else {
        //TODO: Log 
    }

    IOHIDManagerRegisterDeviceMatchingCallback(HIDManager, controllerConnected, (void*)mac_game_controller);

    IOHIDManagerSetDeviceMatchingMultiple(HIDManager, (__bridge CFArrayRef)@[
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)},
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_MultiAxisController)},
    ]);
  
	IOHIDManagerScheduleWithRunLoop(HIDManager, 
                                    CFRunLoopGetMain(), 
                                    kCFRunLoopDefaultMode);
}


// TODO: (Ted)
// 1. Free file memory
// 2. Debug write file

void macRefreshBuffer(Game_Offscreen_Buffer *buffer, NSWindow* Window) {

    if (buffer->memory) {
        free(buffer->memory);
    }

    buffer->width = (uint32)Window.contentView.bounds.size.width;
    buffer->height = (uint32)Window.contentView.bounds.size.height;
    buffer->pitch = buffer->width * buffer->bytes_per_pixel;
    buffer->memory = (uint8 *)malloc(buffer->pitch * buffer->height);
}


// TODO: (Ted)  Replace this with hardware rendering.
void macRedrawBuffer(Game_Offscreen_Buffer* buffer, NSWindow* Window) {
    int size_buffer = buffer->width * buffer->height;
    u32* pixels = (u32*)(buffer->memory);
    for (int i = 0; i < size_buffer; i++) {
        pixels[i] = 0xFF00FF00;
    }
    @autoreleasepool {
        NSBitmapImageRep *Rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes: (unsigned char* _Nullable* _Nullable)buffer->memory
                                  pixelsWide: buffer->width
                                  pixelsHigh: buffer->height
                                  bitsPerSample: 8
                                  samplesPerPixel: 4
                                  hasAlpha: YES
                                  isPlanar: NO
                                  colorSpaceName: NSDeviceRGBColorSpace
                                  bytesPerRow: buffer->pitch
                                  bitsPerPixel: buffer->bytes_per_pixel * 8] autorelease];

        NSSize image_size = NSMakeSize(buffer->width, buffer->height);
        NSImage* image = [[[NSImage alloc] initWithSize: image_size] autorelease];
        [image addRepresentation: Rep];
        Window.contentView.layer.contents = image;
    }
}

static void
macBeginRecordingInput(OSX_State* osx_state)
{
    if (osx_state->replay_memory_block)
    {
        osx_state->recording_handle = osx_state->replay_file_handle;
        fseek(osx_state->recording_handle, (i64)osx_state->permanent_storage_size, SEEK_SET);
        memcpy(osx_state->replay_memory_block, osx_state->game_memory_block, osx_state->permanent_storage_size);
        osx_state->is_recording = true;
    }
}

static void
macEndRecordingInput(OSX_State* osx_state)
{
    osx_state->is_recording = false;
}

static void
macBeginInputPlayback(OSX_State* osx_state)
{
    if (osx_state->replay_memory_block)
    {
        osx_state->playback_handle = osx_state->replay_file_handle;
        fseek(osx_state->playback_handle, (i64)osx_state->permanent_storage_size, SEEK_SET);
        memcpy(osx_state->game_memory_block, osx_state->replay_memory_block, osx_state->permanent_storage_size);
        osx_state->is_playing_back = true;
    }
}

static void
macEndInputPlayback(OSX_State* osx_state)
{
    osx_state->is_playing_back = false;
}

static void
macRecordInput(OSX_State *osx_state, Mac_Game_Controller* new_input)
{
    size_t bytes_written = fwrite(new_input, sizeof(char), sizeof(*new_input), osx_state->recording_handle);
    if (bytes_written <= 0)
    {
        // TODO: (ted) Log Record Input Failure
    }
}

static void
macPlaybackInput(OSX_State* osx_state, Mac_Game_Controller* new_input)
{
    uint64 bytes_read = fread(new_input, sizeof(char), sizeof(*new_input), osx_state->playback_handle);
    if (bytes_read <= 0) 
    {
        macEndInputPlayback(osx_state); 
        macBeginInputPlayback(osx_state);
    }
}


@interface OSX_MainWindowDelegate: NSObject<NSWindowDelegate>
@end

@implementation OSX_MainWindowDelegate 

- (void)WindowWillClose:(id)sender {
    Running = false;  
}

@end

@interface Key_Ignoring_Window: NSWindow
@end

@implementation Key_Ignoring_Window
- (void)keyDown:(NSEvent *)theEvent { }
@end

int main(int argc, const char* argv[]) {

    NSRect screen_rect = [[NSScreen mainScreen] frame];

    real32 global_render_width = 1024;
    real32 global_render_height = 768;

    NSRect initial_frame = NSMakeRect((screen_rect.size.width - (real64)global_render_width) * 0.5,
                                        (screen_rect.size.height - (real64)global_render_height) * 0.5,
                                        (real64)global_render_width, (real64)global_render_height);

    NSWindow *Window = [[Key_Ignoring_Window alloc] 
                         initWithContentRect: initial_frame
                         styleMask: NSWindowStyleMaskTitled |
                                    NSWindowStyleMaskClosable |
                                    NSWindowStyleMaskMiniaturizable 
                         backing: NSBackingStoreBuffered
                         defer: NO];    

            
    [Window setBackgroundColor: NSColor.blackColor];
    [Window setTitle: @"Handmade OSX Trost"];
    [Window makeKeyAndOrderFront: nil];
    // TODO: Delegate
    //[Window setDelegate: MainWindowDelegate];
    Window.contentView.wantsLayer = YES;

   OSX_State Osx_State = {}; 
   Osx_State.is_recording = false;
   Osx_State.is_playing_back = false;

   OSX_AppPath osx_path = {};
   Osx_State.path = osx_path;
   macBuildAppFilePath(&Osx_State.path);

   Game_Offscreen_Buffer buffer = {};
   buffer.bytes_per_pixel = 4;
   macRefreshBuffer(&buffer, Window);

   Game_Memory game_memory = {};
    game_memory.DEBUG_platformReadEntireFile = DEBUGPlatformReadEntireFile;
    game_memory.DEBUG_platformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    game_memory.DEBUG_platformWriteEntireFile = DEBUGPlatformWriteEntireFile;
   
    game_memory.permanent_storage_size = Megabytes(64);
    game_memory.transient_storage_size = Gigabytes(4);

#if HANDMADE_INTERNAL
    char* base_address = (char*)Gigabytes(8);
    i32 allocation_flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED;
#else
    void* base_address = 0;
    i32 allocation_flags = MAP_PRIVATE | MAP_ANON;
#endif 

    i32 access_flags = PROT_READ | PROT_WRITE;

    u64 total_size = game_memory.permanent_storage_size + game_memory.transient_storage_size;

    game_memory.permanent_storage = mmap(base_address, game_memory.permanent_storage_size, access_flags, 
                                       allocation_flags, -1, 0);

    if (game_memory.transient_storage == MAP_FAILED) {
		printf("mmap error: %d  %s", errno, strerror(errno));
        [NSException raise: @"Game Memory Transient Storage Not Allocated"
                     format: @"Failed to allocate transient storage"];
    }

    Osx_State.game_memory_block = game_memory.permanent_storage;
    Osx_State.permanent_storage_size = game_memory.permanent_storage_size;

    int file_descriptor;
    mode_t mode = S_IRUSR | S_IWUSR;
    char filename[MAC_MAX_FILENAME_SIZE];
    char local_filename[MAC_MAX_FILENAME_SIZE];
    // TODO: (Jon) need to figure out waht is going on here.
    //sprintf(local_filename, "Contents/Resources/ReplayBuffer");
    macBuildAppPathFilename(&Osx_State.path, local_filename,
                            sizeof(filename), filename);
    file_descriptor = open(filename, O_CREAT | O_RDWR, mode);
    int result = truncate(filename, (i64)game_memory.permanent_storage_size);

    if (result < 0)
    {
        // TODO: (Jon T)  Log This
    }
    Osx_State.replay_memory_block = mmap(0, game_memory.permanent_storage_size,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE, file_descriptor, 0);
    Osx_State.replay_file_handle = fopen(filename, "r+");

    /*
    fseek(Osx_State.replay_file_handle, (int)Osx_State.permanent_storage_size, SEEK_SET);
    if (Osx_State.replay_memory_block)
    {
    } else {
        // TODO: (casey)    Diagnostic
    }
    */

    OSX_Game_Controller game_controller = {};
    Mac_Game_Controller NewInput = {};
    Mac_Game_Controller OldInput = {};
    // setup game controller.

    OSX_Game_Controller keyboard_controller = {};

    OSX_Game_Controller* mac_controllers[2] = {&keyboard_controller, &game_controller};

    // TODO Game_Input stucts needs to be made

    OSX_SoundOutput sound_output = {};
    // macSetupAudio(&sound_output);

    //TODO: Game_Sound_Output sound_output = {};
    // macSetupAudio(&sound_output);

    // Game_Sound_Output_Buffer sound_buffer = {};
    i16* samples = (i16*)calloc(sound_output.samples_per_second,
                                    sound_output.bytes_per_sample);

    /*
    SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
    uint32 LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
    uint32 TargetQueueBytes = LatencySampleCount * SoundOutput.BytesPerSample;

    local_persist uint32 RunningSampleIndex = 0;

    // TODO: Determine this programmatically.
    int32 MonitorRefreshHz = 60;
    real32 TargetFramesPerSecond = MonitorRefreshHz / 2.0f;
    real32 TargetSecondsPerFrame = 1.0f / TargetFramesPerSecond; 

    char *GameLibraryInBundlePath = (char *)"Contents/Resources/GameCode.dylib";
    char GameLibraryFullPath[MAC_MAX_FILENAME_SIZE];

    MacBuildAppPathFilename(&osx_state.Path, GameLibraryInBundlePath,
                            sizeof(GameLibraryFullPath), GameLibraryFullPath);
    */
    OSX_Game_Code game = {};
    //macLoadGameCode(&game, game_library_full_path);

    mach_timebase_info_data_t TimeBase;
    mach_timebase_info(&TimeBase);

    uint64 last_counter = mach_absolute_time();
    
    while(Running) {
        NSEvent* event;
        do {
            event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                       untilDate: nil
                                          inMode: NSDefaultRunLoopMode
                                         dequeue: YES];
            if (event != nil &&
                (event.type == NSEventTypeKeyDown ||
                event.type == NSEventTypeKeyUp)) 
            {
                //MacHandleKeyboardEvent(&KeyboardController, Event, &osx_state);
            }
            switch ([event type]) {
                default:
                    [NSApp sendEvent: event];
            }
        } while (event != nil);

        Mac_Game_Controller Temp = NewInput;
        NewInput = OldInput;
        OldInput = Temp;
        
        /*
        for (int MacControllerIndex = 0;
             MacControllerIndex < 2;
             MacControllerIndex++)
        {
            mac_game_controller *MacController = MacControllers[MacControllerIndex]; 

            game_controller_input *OldController = &OldInput->Controllers[MacControllerIndex];
            game_controller_input *NewController = &NewInput->Controllers[MacControllerIndex];
            MacProcessGameControllerButton(&(OldController->A),
                                           &(NewController->A),
                                           MacController->CircleButtonState);

            MacProcessGameControllerButton(&(OldController->B),
                                           &(NewController->B),
                                           MacController->XButtonState);

            MacProcessGameControllerButton(&(OldController->X),
                                           &(NewController->X),
                                           MacController->TriangleButtonState);

            MacProcessGameControllerButton(&(OldController->Y),
                                           &(NewController->Y),
                                           MacController->SquareButtonState);

            MacProcessGameControllerButton(&(OldController->LeftShoulder),
                                           &(NewController->LeftShoulder),
                                           MacController->LeftShoulderButtonState);

            MacProcessGameControllerButton(&(OldController->RightShoulder),
                                           &(NewController->RightShoulder),
                                           MacController->RightShoulderButtonState);

            bool32 Right = MacController->DPadX > 0 ? true:false;
            bool32 Left = MacController->DPadX < 0 ? true:false;
            bool32 Up = MacController->DPadY > 0 ? true:false;
            bool32 Down = MacController->DPadY < 0 ? true:false;

            MacProcessGameControllerButton(&(OldController->Right),
                                           &(NewController->Right),
                                           Right);
            MacProcessGameControllerButton(&(OldController->Left),
                                           &(NewController->Left),
                                           Left);
            MacProcessGameControllerButton(&(OldController->Up),
                                           &(NewController->Up),
                                           Up);
            MacProcessGameControllerButton(&(OldController->Down),
                                           &(NewController->Down),
                                           Down);

            // TODO: (Ted)  Figure out if controller really is analog.
            NewController->IsAnalog = true; 

            NewController->StartX = OldController->EndX;
            NewController->StartY = OldController->EndY;

            NewController->EndX = (real32)(MacController->LeftThumbstickX - 127.5f)/127.5f;
            NewController->EndY = (real32)(MacController->LeftThumbstickY - 127.5f)/127.5f;

            NewController->MinX = NewController->MaxX = NewController->EndX;
            NewController->MinY = NewController->MaxY = NewController->EndY;
        }
        */

    // Sound stuff
    /*
    uint32 TargetCursor = ((SoundOutput.PlayCursor + TargetQueueBytes) % SoundOutput.BufferSize);

    uint32 ByteToLock = (RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.BufferSize; 
    uint32 BytesToWrite;

        if (ByteToLock > TargetCursor) {
        // NOTE: (ted)  Play Cursor wrapped.

        // Bytes to the end of the circular buffer.
        BytesToWrite = (SoundOutput.BufferSize - ByteToLock);

        // Bytes up to the target cursor.
        BytesToWrite += TargetCursor;
    } else {
        BytesToWrite = TargetCursor - ByteToLock;
    }

    // NOTE: (Ted)  This is where we can calculate the number of sound samples to write
    //              to the game_sound_output_buffer
    SoundBuffer.Samples = Samples;
    SoundBuffer.SampleCount = (BytesToWrite/SoundOutput.BytesPerSample);
    */
    /*
        if (game.DLLLastWriteTime < NewGameLibraryWriteTime)
        {
            MacUnloadGameCode(&game); 
            MacLoadGameCode(&game, GameLibraryFullPath); 
        }
        */

        if (Osx_State.is_recording)
        {
            macRecordInput(&Osx_State, &NewInput);
        } else if (Osx_State.is_playing_back)
        {
            macPlaybackInput(&Osx_State, &NewInput);
        }

        //game.UpdateAndRender(&game_memory, NewInput, &buffer);
        //game.GetSoundSamples(&game_memory, &sound_buffer);

/*
        void *Region1 = (uint8*)SoundOutput.Data + ByteToLock;
        uint32 Region1Size = BytesToWrite;
        
        if (Region1Size + ByteToLock > SoundOutput.BufferSize) {
            Region1Size = SoundOutput.BufferSize - ByteToLock;
        }

        void *Region2 = SoundOutput.Data;
        uint32 Region2Size = BytesToWrite - Region1Size;

        uint32 Region1SampleCount = Region1Size/SoundOutput.BytesPerSample;
        int16* SampleOut = (int16*)Region1;

        for (uint32 SampleIndex = 0;
             SampleIndex < Region1SampleCount;
             ++SampleIndex) {
            *SampleOut++ = *SoundBuffer.Samples++;
            *SampleOut++ = *SoundBuffer.Samples++;
            RunningSampleIndex++;
        }

        uint32 Region2SampleCount = Region2Size/SoundOutput.BytesPerSample;
        SampleOut = (int16*)Region2;
       
        for (uint32 SampleIndex = 0;
             SampleIndex < Region2SampleCount;
             ++SampleIndex) {
            *SampleOut++ = *SoundBuffer.Samples++;
            *SampleOut++ = *SoundBuffer.Samples++;
            RunningSampleIndex++;
        }

        uint64 WorkCounter = mach_absolute_time();

*/
        //real32 WorkSeconds = MacGetSecondsElapsed(&TimeBase, LastCounter, WorkCounter);

        //real32 SecondsElapsedForFrame = WorkSeconds;

/*
        if (SecondsElapsedForFrame < TargetSecondsPerFrame)
        {
            // NOTE: We need to sleep up to the target framerate 

            real32 UnderOffset = 3.0f / 1000.0f;
            real32 SleepTime = TargetSecondsPerFrame - SecondsElapsedForFrame - UnderOffset;
            useconds_t SleepMS = (useconds_t)(1000.0f * 1000.0f * SleepTime);
            
            if (SleepMS > 0)
            {
                usleep(SleepMS);
            }

            while (SecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                SecondsElapsedForFrame = MacGetSecondsElapsed(&TimeBase, LastCounter,
                                                              mach_absolute_time());
            }

        } else
        {
            // TODO: Log MISSED FRAME RATE!!!
        }
        */
    
        // NOTE: End of Updates
        uint64 EndOfFrameTime = mach_absolute_time();

        //uint64 TimeUnitsPerFrame = EndOfFrameTime - LastCounter;

        // Here is where you print stuff..
        //uint64 NanosecondsPerFrame = TimeUnitsPerFrame * (TimeBase.numer / TimeBase.denom);
        //real32 SecondsPerFrame = (real32)NanosecondsPerFrame * (real32)1.0E-9;
        //real32 MillesSecondsPerFrame = (real32)NanosecondsPerFrame * (real32)1.0E-6;
        //real32 FramesPerSecond = 1 / SecondsPerFrame;

        //NSLog(@"Frames Per Second: %f", (real64)FramesPerSecond); 
        //NSLog(@"MillesSecondsPerFrame: %f", (real64)MillesSecondsPerFrame); 

        //LastCounter = mach_absolute_time();

        macRedrawBuffer(&buffer, Window); 
    }
}
