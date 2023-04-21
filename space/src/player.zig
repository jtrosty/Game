const std = @import("std");
const rl = @import("raylib");
const rlm = @import("raylib-math");
const gmath = @import("math.zig");
const entity = @import("Entity.zig");

const SHIP_HEIGHT = 40.0;
pub const MAX_BULLETS = 15;

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
    collider: rl.Vector3, // The collider will be a cirlce, x, y position, and z is the radius.
    color: rl.Color,
};

var player_one = Player{
    .ship_height = SHIP_HEIGHT,
    .gun_length = 15,
    .position = rl.Vector2{ .x = gmath.screenWidth / 2.0, .y = gmath.screenHeight / 2.0 },
    .acceleartion = 0.0,
    .speed = rl.Vector2{ .x = 0.0, .y = 0.0 },
    .speed_scale = 3.0,
    .rotation = 0.0,
    .gun_direction = rl.Vector2{ .x = 0.0, .y = 0.0 },
    .gun_rotation = 0.0,
    .collider = rl.Vector3{
        .x = (gmath.screenWidth / 2) + @sin(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5),
        .y = (gmath.screenHeight / 2) - @cos(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5),
        .z = 12,
    },
    .color = rl.RED,
};

pub fn init_player() void {
    player_one = Player{
        .ship_height = SHIP_HEIGHT,
        .gun_length = 15,
        .position = rl.Vector2{ .x = gmath.screenWidth / 2.0, .y = gmath.screenHeight / 2.0 },
        .acceleartion = 0.0,
        .speed = rl.Vector2{ .x = 0.0, .y = 0.0 },
        .speed_scale = 3.0,
        .rotation = 0.0,
        .gun_direction = rl.Vector2{ .x = 0.0, .y = 0.0 },
        .gun_rotation = 0.0,
        .collider = rl.Vector3{
            .x = (gmath.screenWidth / 2) + @sin(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5),
            .y = (gmath.screenHeight / 2) - @cos(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5),
            .z = 12,
        },
        .color = rl.RED,
    };
}

pub fn render_player() void {
    const v1_front = rl.Vector2{ .x = player_one.position.x + @sin(player_one.rotation) * (player_one.ship_height), .y = player_one.position.y - @cos(player_one.rotation) * (player_one.ship_height) };
    const v2_left = rl.Vector2{ .x = player_one.position.x - @cos(player_one.rotation) * (player_one.ship_height / 3), .y = player_one.position.y - @sin(player_one.rotation) * (player_one.ship_height / 3) };
    const v3_right = rl.Vector2{ .x = player_one.position.x + @cos(player_one.rotation) * (player_one.ship_height / 3), .y = player_one.position.y + @sin(player_one.rotation) * (player_one.ship_height / 3) };

    const v_rear_gun_center_point = rl.Vector2{ .x = player_one.position.x + @sin(player_one.rotation) * (player_one.ship_height / 6), .y = player_one.position.y - @cos(player_one.rotation) * (player_one.ship_height / 6) };

    const v_gun_center_base = rl.Vector2{ .x = player_one.position.x + @sin(player_one.rotation) * (player_one.ship_height / 2), .y = player_one.position.y - @cos(player_one.rotation) * (player_one.ship_height / 2) };
    const v_gun_rear_1_base = rl.Vector2{ .x = v_rear_gun_center_point.x - @cos(player_one.rotation) * (player_one.ship_height / 5), .y = v_rear_gun_center_point.y - @sin(player_one.rotation) * (player_one.ship_height / 5) };
    const v_gun_rear_2_base = rl.Vector2{ .x = v_rear_gun_center_point.x + @cos(player_one.rotation) * (player_one.ship_height / 5), .y = v_rear_gun_center_point.y + @sin(player_one.rotation) * (player_one.ship_height / 5) };

    const v_gun_center_distal = rl.Vector2{ .x = v_gun_center_base.x + @cos(player_one.gun_rotation) * player_one.gun_length, .y = v_gun_center_base.y + @sin(player_one.gun_rotation) * player_one.gun_length };
    const v_gun_rear_1_distal = rl.Vector2{ .x = v_gun_rear_1_base.x + @cos(player_one.gun_rotation) * player_one.gun_length, .y = v_gun_rear_1_base.y + @sin(player_one.gun_rotation) * player_one.gun_length };
    const v_gun_rear_2_distal = rl.Vector2{ .x = v_gun_rear_2_base.x + @cos(player_one.gun_rotation) * player_one.gun_length, .y = v_gun_rear_2_base.y + @sin(player_one.gun_rotation) * player_one.gun_length };
    rl.DrawTriangle(v1_front, v2_left, v3_right, player_one.color);
    rl.DrawLineEx(v_gun_rear_1_base, v_gun_rear_1_distal, 3, rl.GREEN);
    rl.DrawLineEx(v_gun_rear_2_base, v_gun_rear_2_distal, 3, rl.BLACK);
    rl.DrawLineEx(v_gun_center_base, v_gun_center_distal, 3, rl.BLUE);
}

pub fn render_bullets(bullets: []entity.Entity) void {
    // DRAW THE BULLETS
    for (bullets) |bullet| {
        if (bullet.bullet_type == entity.Entity_Type.none) {
            continue;
        }
        // Draw the bullet.
        else if (bullet.bullet_type == entity.Entity_Type.normal_bullet) {
            rl.DrawCircleV(bullet.position, bullet.size, rl.YELLOW);
        }
    }
}

pub fn controller_player(bullets: []entity.Entity) void {
    if (rl.IsGamepadAvailable(0)) {} else {
        std.debug.print("gamepad not available.", .{});
    }
    if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE)) {}
    // Draw buttons: basic
    if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE_RIGHT)) {}
    if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_MIDDLE_LEFT)) {}
    if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
        player_one.color = rl.YELLOW;
    }
    if (rl.IsGamepadButtonDown(0, rl.GamepadButton.GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
        player_one.color = rl.BLUE;
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
        player_one.color = rl.YELLOW;
    }
    if (rl.IsKeyDown(rl.KeyboardKey.KEY_Q)) {
        player_one.color = rl.BLUE;
    }
    if (rl.IsKeyDown(rl.KeyboardKey.KEY_R)) {
        player_one.color = rl.GREEN;
    }
    if (rl.IsKeyDown(rl.KeyboardKey.KEY_SPACE)) {
        for (bullets) |bullet| {
            // Gen bullet
            if (bullet.bullet_type == entity.Entity_Type.none) {
                // TODO(Jon): Generate the bullet.

            }
        }
    }

    var keyboard_x_input: f32 = 0.0;
    var keyboard_y_input: f32 = 0.0;
    if (rl.IsKeyDown(rl.KeyboardKey.KEY_W)) {
        keyboard_y_input = -1.0;
    } else if (rl.IsKeyDown(rl.KeyboardKey.KEY_S)) {
        keyboard_y_input = 1.0;
    } else {
        keyboard_y_input = 0.0;
    }
    if (rl.IsKeyDown(rl.KeyboardKey.KEY_A)) {
        keyboard_x_input = -1.0;
    } else if (rl.IsKeyDown(rl.KeyboardKey.KEY_D)) {
        keyboard_x_input = 1.0;
    } else {
        keyboard_x_input = 0.0;
    }

    const gamepad_dir_left: rl.Vector2 = rl.Vector2{ .x = gamepad_axis_LX, .y = gamepad_axis_LY };
    const gamepad_dir_right: rl.Vector2 = rl.Vector2{ .x = gamepad_axis_RX, .y = gamepad_axis_RY };

    // TODO(Jon): Make a constant somewhere
    const analog_stick_floor = 0.2;
    const gamepad_vector_length_left = rlm.Vector2Length(gamepad_dir_left);
    const gamepad_vector_length_right = rlm.Vector2Length(gamepad_dir_right);

    if ((keyboard_x_input != 0.0) or (keyboard_y_input != 0.0)) {
        player_one.position.x += (keyboard_x_input * player_one.speed_scale);
        player_one.position.y += (keyboard_y_input * player_one.speed_scale);
    } else {
        player_one.position.x += (gamepad_axis_LX * player_one.speed_scale);
        player_one.position.y += (gamepad_axis_LY * player_one.speed_scale);
    }
    // TODO: (Jon)  Position has been updated, update colider

    //if ((@fabs(gamepad_axis_LX) > analog_stick_floor) and (@fabs(gamepad_axis_LY) > analog_stick_floor)) {
    if (gamepad_vector_length_left > analog_stick_floor) {
        // Need the negative to get the roation to be correct
        player_one.rotation = -std.math.atan2(f32, gamepad_axis_LX, gamepad_axis_LY);
        player_one.rotation += gmath.PI;
    } else {
        // Do nothing if they are zero
    }
    // Update collision position
    player_one.collider.x = (gmath.screenWidth / 2) + @sin(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5);
    player_one.collider.y = (gmath.screenHeight / 2) - @cos(0.0 * gmath.DEG2RAD) * (SHIP_HEIGHT / 2.5);

    // TODO(Jon): Mouse is bug but in generla it does what I need it to do.
    if (rl.IsMouseButtonDown(rl.MouseButton.MOUSE_BUTTON_LEFT)) {
        const mouse_position: rl.Vector2 = rl.Vector2{ .x = rl.GetMousePosition().x, .y = rl.GetMousePosition().y };
        player_one.gun_rotation = -std.math.atan2(f32, mouse_position.x - player_one.position.x, mouse_position.y - player_one.position.y);
        player_one.gun_rotation += gmath.PI / 2;
    } else {
        if (gamepad_vector_length_right > analog_stick_floor) {
            // Need the negative to get the roation to be correct
            player_one.gun_rotation = -std.math.atan2(f32, gamepad_axis_RX, gamepad_axis_RY);
            player_one.gun_rotation += gmath.PI / 2;
        } else {
            // Do nothing if they are zero
        }
    }
}
