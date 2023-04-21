const rl = @import("raylib");

pub const Entity_Type = enum(u8) {
    none,
    normal_bullet,
};

pub const Entity = struct {
    bullet_type: Entity_Type = Entity_Type.none,
    position: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    speed: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    gun_direction: rl.Vector2 = rl.Vector2{ .x = 0, .y = 0 },
    speed_scale: f32 = 0.0,
    rotation: f32 = 0.0,
    collider: rl.Vector3 = rl.Vector3{ .x = 0, .y = 0, .z = 0 },
    size: f32 = 0.0,
};
