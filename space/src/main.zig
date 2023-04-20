//
// basic_window
// Zig version: 0.6.0
// Author: Nikolas Wipper
// Date: 2020-02-15
//

const std = @import("std");
const rl = @import("raylib");
const rlm = @import("raylib-math");
const player = @import("player.zig");
const entity = @import("Entity.zig");
const Allocator = std.mem.Allocator;
const assert = std.debug.assert;

pub fn main() anyerror!void {
    // Initialization
    //--------------------------------------------------------------------------------------
    const screenWidth = 1600;
    const screenHeight = 900;

    rl.InitWindow(screenWidth, screenHeight, "raylib-zig [core] example - basic window");

    rl.SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!rl.WindowShouldClose()) { // Detect window close button or ESC key
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------
        player.controller_player();


        // Draw
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------
        rl.BeginDrawing();

        rl.ClearBackground(rl.GRAY);
        //rl.DrawCircleV(player_one.position, 9, rl.RED);
        // DRAWING THE CHARACTER
        rl.DrawText("Congrats! You created your first window!", 190, 200, 20, rl.LIGHTGRAY);

        player.render_player();
        player.render_bullets();

        // Debug
        // GAME PAD DEBUG--------------------------------------------------------------------------------

        rl.EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    rl.CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}
