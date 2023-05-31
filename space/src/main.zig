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

    const ArrayList = std.ArrayList;
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    var bullet_array = ArrayList(entity.Entity).init(allocator);
    var test_bullet_array: []entity.Entity = allocator.alloc(entity.Entity, 10);

    var i: u16 = 0;
    var k: f32 = 0.0;
    while (i < 10) {
        //bullet_array.append(entity.Entity);
        //bullet_array[i].bullet_type = entity.Entity_Type.normal_bullet;
        //bullet_array[i].position = rl.Vector2{ .x = 5 * i, .y = 5 * i };
        //bullet_array[i].size = 5.0;
        test_bullet_array[i].bullet_type = entity.Entity_Type.normal_bullet;
        test_bullet_array[i].position = rl.Vector2{ .x = 5 * k, .y = 5 * k };
        test_bullet_array[i].size = 5.0;
        k += 1.0;
        i += 1;
    }

    // Main game loop
    while (!rl.WindowShouldClose()) { // Detect window close button or ESC key
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        //----------------------------------------------------------------------------------
        player.controller_player(bullet_array);

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
        player.render_bullets(bullet_array);

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
