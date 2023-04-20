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
const Allocator = std.mem.Allocator;
const assert = std.debug.assert;

const MAX_BULLETS = 15;

const Entity_Type = enum {
    none,
    normal_bullet,
};

const Bullet = struct {
    bullet_type: Entity_Type = Entity_Type.none,
    position: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    speed: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    gun_direction: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    speed_scale: f32 = 0.0,
    rotation: f32 = 0.0,
    collider: rl.Vector3 = rl.Vector3{ .x = 0, .y = 0, .z = 0 },
    size: f32 = 0.0,
};

const Particle = struct {
    position: rl.Vector2,
    color: rl.Color,
    //life,
    // Particle type
};

pub fn DrawParticle(p: *Particle) void {
    rl.DrawCircleV(p.position.x, p.position.y, 5, p.color);
}

const PI: f32 = 3.141592653589;
const DEG2RAD: f32 = PI / 180.0;

pub fn main() anyerror!void {
    // Initialization
    //--------------------------------------------------------------------------------------
    const screenWidth = 1600;
    const screenHeight = 900;
    var bullet_array = std.mem.zeroes([MAX_BULLETS]Bullet);


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

        rl.BeginDrawing();

        rl.ClearBackground(rl.GRAY);
        //rl.DrawCircleV(player_one.position, 9, rl.RED);
        // DRAWING THE CHARACTER
        rl.DrawText("Congrats! You created your first window!", 190, 200, 20, rl.LIGHTGRAY);

        // DRAW THE BULLETS
        for (bullet_array) |bullet| {
            if (bullet.bullet_type == Entity_Type.none) {
                continue;
            }
            // Draw the bullet.
            else if (bullet.bullet_type == Entity_Type.normal_bullet) {
                rl.DrawCircleV(bullet.position, bullet.size, rl.YELLOW);
            }
        }
        player.render_player();

        // Debug
        // GAME PAD DEBUG--------------------------------------------------------------------------------


        if (rl.GetGamepadButtonPressed() != 0) {
            rl.DrawText(rl.TextFormat("DETECTED BUTTON: %i", rl.GetGamepadButtonPressed()), 10, 430, 10, rl.RED);
        } else rl.DrawText("DETECTED BUTTON: NONE", 10, 430, 10, rl.GRAY);

        rl.EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    rl.CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}
