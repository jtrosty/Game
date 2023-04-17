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
    ship_height: f32,
    gun_length: f32,
    position: rl.Vector2,
    speed: rl.Vector2,
    speed_scale: f32,
    acceleartion: f32,
    rotation: f32,
    gun_direction: rl.Vector2,
    gun_rotation: f32,
    collider: rl.Vector3,
    color: rl.Color,
};

const Bullet = struct {
    ship_height: f32,
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
    const screenWidth = 1600;
    const screenHeight = 900;
    const SHIP_HEIGHT = 40.0;

    var player = Player{
        .ship_height = SHIP_HEIGHT,
        .gun_length = 15,
        .position = rl.Vector2{ .x = screenWidth / 2.0, .y = screenHeight / 2.0 },
        .acceleartion = 0.0,
        .speed = rl.Vector2{ .x = 0.0, .y = 0.0 },
        .speed_scale = 3.0,
        .rotation = 0.0,
        .gun_direction = rl.Vector2{ .x = 0.0, .y = 0.0 },
        .gun_rotation = 0.0,
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

        // Left
        var gamepad_axis_LX: f32 = rl.GetGamepadAxisMovement(0, 0);
        var gamepad_axis_LY: f32 = rl.GetGamepadAxisMovement(0, 1);
        // Right
        var gamepad_axis_RX: f32 = rl.GetGamepadAxisMovement(0, 2);
        var gamepad_axis_RY: f32 = rl.GetGamepadAxisMovement(0, 3);

        // KEYBOARD
        // E F R
        if (rl.IsKeyDown(rl.KeyboardKey.KEY_E)) {
            player.color = rl.YELLOW;
        }
        if (rl.IsKeyDown(rl.KeyboardKey.KEY_Q)) {
            player.color = rl.BLUE;
        }
        if (rl.IsKeyDown(rl.KeyboardKey.KEY_R)) {
            player.color = rl.GREEN;
        }

        var keyboard_x_input: f32 = 0.0;
        var keyboard_y_input: f32 = 0.0;
        if (rl.IsKeyDown(rl.KeyboardKey.KEY_W)) {
            keyboard_y_input = -1.0;
        }
        else if (rl.IsKeyDown(rl.KeyboardKey.KEY_S)) {
            keyboard_y_input = 1.0;
        }
        else {
            keyboard_y_input = 0.0;
        }
        if (rl.IsKeyDown(rl.KeyboardKey.KEY_A)) {
            keyboard_x_input = -1.0;
        }
        else if (rl.IsKeyDown(rl.KeyboardKey.KEY_D)) {
            keyboard_x_input = 1.0;
        }
        else {
            keyboard_x_input = 0.0;
        }

        const gamepad_dir_left: rl.Vector2 = rl.Vector2{ .x = gamepad_axis_LX, .y = gamepad_axis_LY };
        const gamepad_dir_right: rl.Vector2 = rl.Vector2{ .x = gamepad_axis_RX, .y = gamepad_axis_RY };

        // TODO(Jon): Make a constant somewhere
        const analog_stick_floor = 0.2;
        const gamepad_vector_length_left = rlm.Vector2Length(gamepad_dir_left);
        const gamepad_vector_length_right = rlm.Vector2Length(gamepad_dir_right);

        if ((keyboard_x_input != 0.0) or (keyboard_y_input != 0.0)) {
            player.position.x += (keyboard_x_input * player.speed_scale);
            player.position.y += (keyboard_y_input * player.speed_scale);
        }
        else {
            player.position.x += (gamepad_axis_LX * player.speed_scale);
            player.position.y += (gamepad_axis_LY * player.speed_scale);
        }
        
        //if ((@fabs(gamepad_axis_LX) > analog_stick_floor) and (@fabs(gamepad_axis_LY) > analog_stick_floor)) {
        if (gamepad_vector_length_left > analog_stick_floor) {
            // Need the negative to get the roation to be correct
            player.rotation = -std.math.atan2(f32, gamepad_axis_LX, gamepad_axis_LY);
            player.rotation += PI;
        } else {
            // Do nothing if they are zero
        }

        if (gamepad_vector_length_right > analog_stick_floor) {
            // Need the negative to get the roation to be correct
            player.gun_rotation = -std.math.atan2(f32, gamepad_axis_RX, gamepad_axis_RY);
            player.gun_rotation += PI / 2;
        } else {
            // Do nothing if they are zero
        }

        //TODO: Detele the commented things
        //const dirVect = rl.Vector2{ .x = debug_gamepad_axis_LX, .y = debug_gamepadAxisLY };
        //player.rotation = rlm.Vector2Angle(zeroPos, dirVect);
        std.debug.print("player rot: {}\n", .{player.gun_rotation});
        gamepad_axis_LX = 1.0;

        //----------------------------------------------------------------------------------
        rl.BeginDrawing();

        rl.ClearBackground(rl.GRAY);
        //rl.DrawCircleV(player.position, 9, rl.RED);
        const v1_front = rl.Vector2{ .x = player.position.x + @sin(player.rotation) * (player.ship_height), .y = player.position.y - @cos(player.rotation) * (player.ship_height) };
        const v2_left = rl.Vector2{ .x = player.position.x - @cos(player.rotation) * (player.ship_height / 3), .y = player.position.y - @sin(player.rotation) * (player.ship_height / 3) };
        const v3_right = rl.Vector2{ .x = player.position.x + @cos(player.rotation) * (player.ship_height / 3), .y = player.position.y + @sin(player.rotation) * (player.ship_height / 3) };

        const v_rear_gun_center_point = rl.Vector2{ .x = player.position.x + @sin(player.rotation) * (player.ship_height / 6), .y = player.position.y - @cos(player.rotation) * (player.ship_height / 6) };

        const v_gun_center_base = rl.Vector2{ .x = player.position.x + @sin(player.rotation) * (player.ship_height / 2), .y = player.position.y - @cos(player.rotation) * (player.ship_height / 2) };
        const v_gun_rear_1_base = rl.Vector2{ .x = v_rear_gun_center_point.x - @cos(player.rotation) * (player.ship_height / 5), .y = v_rear_gun_center_point.y - @sin(player.rotation) * (player.ship_height / 5) };
        const v_gun_rear_2_base = rl.Vector2{ .x = v_rear_gun_center_point.x + @cos(player.rotation) * (player.ship_height / 5), .y = v_rear_gun_center_point.y + @sin(player.rotation) * (player.ship_height / 5) };

        const v_gun_center_distal = rl.Vector2{ .x = v_gun_center_base.x + @cos(player.gun_rotation) * player.gun_length, .y = v_gun_center_base.y + @sin(player.gun_rotation) * player.gun_length };
        const v_gun_rear_1_distal = rl.Vector2{ .x = v_gun_rear_1_base.x + @cos(player.gun_rotation) * player.gun_length, .y = v_gun_rear_1_base.y + @sin(player.gun_rotation) * player.gun_length };
        const v_gun_rear_2_distal = rl.Vector2{ .x = v_gun_rear_2_base.x + @cos(player.gun_rotation) * player.gun_length, .y = v_gun_rear_2_base.y + @sin(player.gun_rotation) * player.gun_length };
        rl.DrawTriangle(v1_front, v2_left, v3_right, player.color);
        rl.DrawLineEx(v_gun_rear_1_base, v_gun_rear_1_distal, 3, rl.GREEN);
        rl.DrawLineEx(v_gun_rear_2_base, v_gun_rear_2_distal, 3, rl.BLACK);
        rl.DrawLineEx(v_gun_center_base, v_gun_center_distal, 3, rl.BLUE);
        rl.DrawText("Congrats! You created your first window!", 190, 200, 20, rl.LIGHTGRAY);

        // Debug
        // GAME PAD DEBUG--------------------------------------------------------------------------------
        const debug_axis = [_]u8{ 0, 1, 2, 3 };
        rl.DrawText(rl.TextFormat("DETECTED AXIS [%i]:", rl.GetGamepadAxisCount(0)), 10, 50, 10, rl.MAROON);

        rl.DrawText(rl.TextFormat("LX AXIS %i: %.2f", debug_axis[0], gamepad_axis_LX), 20, 70 + 20 * 0, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("LY AXIS %i: %.2f", debug_axis[1], gamepad_axis_LY), 20, 70 + 20 * 1, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("RX AXIS %i: %.2f", debug_axis[2], gamepad_axis_RX), 20, 70 + 20 * 2, 10, rl.DARKGRAY);
        rl.DrawText(rl.TextFormat("RY AXIS %i: %.2f", debug_axis[3], gamepad_axis_RY), 20, 70 + 20 * 3, 10, rl.DARKGRAY);
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
