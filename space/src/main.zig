//
// basic_window
// Zig version: 0.6.0
// Author: Nikolas Wipper
// Date: 2020-02-15
//

const std = @import("std");
const rl = @import("raylib");
const rlm = @import("raylib-math");

const Player = struct {
    shipHeight: f32,
    position: rl.Vector2,
    speed: rl.Vector2,
    acceleartion: f32,
    rotation: f32,
    collider: rl.Vector3,
    color: rl.Color,
};

const Bullet = struct {
    shipHeight: f32,
};

const Particle = struct {
    position: rl.Vector2,
    color: rl.Color,
    //life,
    // Particle type
};

pub fn DrawParticle(p: *Particle) void {
    rl.DrawCircle(p.position.x, p.position.y, 5, p.color);
}

const PI: f32 = 3.141592653589;
const DEG2RAD: f32 = PI / 180.0;

pub fn main() anyerror!void {
    // Initialization
    //--------------------------------------------------------------------------------------
    const screenWidth = 800;
    const screenHeight = 450;
    const SHIP_HEIGHT = 10.0;

    var player = Player{
        .shipHeight = SHIP_HEIGHT,
        .position = rl.Vector2{ .x = screenWidth / 2.0, .y = screenHeight / 2.0 },
        .acceleartion = 0.0,
        .speed = rl.Vector2{ .x = 0.0, .y = 0.0 },
        .rotation = 0.0,
        .collider = rl.Vector3{
            .x = (screenWidth / 2) + @sin(0.0 * DEG2RAD) * (SHIP_HEIGHT / 2.5),
            .y = (screenHeight / 2) - @cos(0.0 * DEG2RAD) * (SHIP_HEIGHT / 2.5),
            .z = 12,
        },
        .color = rl.RED,
    };

    rl.InitWindow(screenWidth, screenHeight, "raylib-zig [core] example - basic window");

    rl.SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!rl.WindowShouldClose()) { // Detect window close button or ESC key
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        if (rl.IsGamepadAvailable(0)) {} else {
            std.debug.print("gamepad not available.", .{});
        }

        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE)) {}

        // Draw buttons: basic
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE_RIGHT)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE_LEFT)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
            player.color = rl.YELLOW;
        }
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            player.color = rl.BLUE;
        }
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_UP)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_UP)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_LEFT_TRIGGER_1)) {}
        if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_TRIGGER_1)) {}

        // Draw axis: left joystick
        const movementMultiplier = 10.0;
        var debug_gamepadAxisLX: f32 = rl.GetGamepadAxisMovement(0, 0) * movementMultiplier;
        var debug_gamepadAxisLY: f32 = rl.GetGamepadAxisMovement(0, 1) * movementMultiplier;
        var debug_gamepadAxisRX: f32 = rl.GetGamepadAxisMovement(0, 2) * movementMultiplier;
        var debug_gamepadAxisRY: f32 = rl.GetGamepadAxisMovement(0, 3) * movementMultiplier;
        player.position.x += debug_gamepadAxisLX;
        player.position.y += debug_gamepadAxisLY;
        const zeroPos = rl.Vector2{ .x = 0, .y = 0 };
        const dirVect = rl.Vector2{ .x = debug_gamepadAxisLX, .y = debug_gamepadAxisLY };
        player.rotation = rlm.Vector2Angle(zeroPos, dirVect);
        debug_gamepadAxisLX = 1.0;

        //----------------------------------------------------------------------------------
        rl.BeginDrawing();

        rl.ClearBackground(rl.WHITE);
        //rl.DrawCircleV(player.position, 9, rl.RED);
        const v1 = rl.Vector2{ .x = player.position.x + @sin(player.rotation * DEG2RAD) * (player.shipHeight), .y = player.position.y - @cos(player.rotation * DEG2RAD) * (player.shipHeight) };
        const v2 = rl.Vector2{ .x = player.position.x - @cos(player.rotation * DEG2RAD) * (player.shipHeight / 2), .y = player.position.y - @sin(player.rotation * DEG2RAD) * (player.shipHeight / 2) };
        const v3 = rl.Vector2{ .x = player.position.x + @cos(player.rotation * DEG2RAD) * (player.shipHeight / 2), .y = player.position.y + @sin(player.rotation * DEG2RAD) * (player.shipHeight / 2) };
        rl.DrawTriangle(v1, v2, v3, player.color);
        rl.DrawText("Congrats! You created your first window!", 190, 200, 20, rl.LIGHTGRAY);

        // Debug
        // GAME PAD DEBUG--------------------------------------------------------------------------------
        const debug_axis = [_]u8{ 0, 1, 2, 3 };
        rl.DrawText(rl.TextFormat("DETECTED AXIS [%i]:", rl.GetGamepadAxisCount(0)), 10, 50, 10, rl.MAROON);

        rl.DrawText(rl.TextFormat("LX AXIS %i: %.2f", debug_axis[0], debug_gamepadAxisLX), 20, 70 + 20 * 0, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("LY AXIS %i: %.2f", debug_axis[1], debug_gamepadAxisLY), 20, 70 + 20 * 1, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("RX AXIS %i: %.2f", debug_axis[2], debug_gamepadAxisRX), 20, 70 + 20 * 2, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("RY AXIS %i: %.2f", debug_axis[3], debug_gamepadAxisRY), 20, 70 + 20 * 3, 10, rl.DARKGRAY);
        //std.debug.print("Left Xaxis: {d}\n", .{debug_gamePadAxisLX});
        //
        //        for (debug_axis) |i| {
        //            rl.DrawText(rl.TextFormat("AXIS %i: %.02f", i, rl.GetGamepadAxisMovement(0, i)), 20, 70 + 20 * i, 10, rl.DARKGRAY);
        //        }

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
