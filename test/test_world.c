// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "constants.h"
#include "core.h"
#include "test_macros.h"

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "box2d/math_functions.h"

#include <stdio.h>

// This is a simple example of building and running a simulation
// using Box2D. Here we create a large ground box and a small dynamic
// box.
// There are no graphics for this example. Box2D is meant to be used
// with your rendering engine in your game engine.
int HelloWorld( void )
{
	// Construct a world object, which will hold and simulate the rigid bodies.
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = ( b2Vec2 ){ 0.0f, -10.0f };

	b2WorldId worldId = b2CreateWorld( &worldDef );
	ENSURE( b2World_IsValid( worldId ) );

	// Define the ground body.
	b2BodyDef groundBodyDef = b2DefaultBodyDef();
	groundBodyDef.position = ( b2Vec2 ){ 0.0f, -10.0f };

	// Call the body factory which allocates memory for the ground body
	// from a pool and creates the ground box shape (also from a pool).
	// The body is also added to the world.
	b2BodyId groundId = b2CreateBody( worldId, &groundBodyDef );
	ENSURE( b2Body_IsValid( groundId ) );

	// Define the ground box shape. The extents are the half-widths of the box.
	b2Polygon groundBox = b2MakeBox( 50.0f, 10.0f );

	// Add the box shape to the ground body.
	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape( groundId, &groundShapeDef, &groundBox );

	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = ( b2Vec2 ){ 0.0f, 4.0f };

	b2BodyId bodyId = b2CreateBody( worldId, &bodyDef );

	// Define another box shape for our dynamic body.
	b2Polygon dynamicBox = b2MakeBox( 1.0f, 1.0f );

	// Define the dynamic body shape
	b2ShapeDef shapeDef = b2DefaultShapeDef();

	// Set the box density to be non-zero, so it will be dynamic.
	shapeDef.density = 1.0f;

	// Override the default friction.
	shapeDef.material.friction = 0.3f;

	// Add the shape to the body.
	b2CreatePolygonShape( bodyId, &shapeDef, &dynamicBox );

	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 4 sub-steps. This provides a high quality simulation
	// in most game scenarios.
	float timeStep = 1.0f / 60.0f;
	int subStepCount = 4;

	b2Vec2 position = b2Body_GetPosition( bodyId );
	b2Rot rotation = b2Body_GetRotation( bodyId );

	// This is our little game loop.
	for ( int i = 0; i < 90; ++i )
	{
		// Instruct the world to perform a single step of simulation.
		// It is generally best to keep the time step and iterations fixed.
		b2World_Step( worldId, timeStep, subStepCount );

		// Now print the position and angle of the body.
		position = b2Body_GetPosition( bodyId );
		rotation = b2Body_GetRotation( bodyId );

		// printf("%4.2f %4.2f %4.2f\n", position.x, position.y, b2Rot_GetAngle(rotation));
	}

	// When the world destructor is called, all bodies and joints are freed. This can
	// create orphaned ids, so be careful about your world management.
	b2DestroyWorld( worldId );

	ENSURE( b2AbsFloat( position.x ) < 0.01f );
	ENSURE( b2AbsFloat( position.y - 1.00f ) < 0.01f );
	ENSURE( b2AbsFloat( b2Rot_GetAngle( rotation ) ) < 0.01f );

	return 0;
}

int EmptyWorld( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2WorldId worldId = b2CreateWorld( &worldDef );
	ENSURE( b2World_IsValid( worldId ) == true );

	float timeStep = 1.0f / 60.0f;
	int32_t subStepCount = 1;

	for ( int32_t i = 0; i < 60; ++i )
	{
		b2World_Step( worldId, timeStep, subStepCount );
	}

	b2DestroyWorld( worldId );

	ENSURE( b2World_IsValid( worldId ) == false );

	return 0;
}

#define BODY_COUNT 10
int DestroyAllBodiesWorld( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2WorldId worldId = b2CreateWorld( &worldDef );
	ENSURE( b2World_IsValid( worldId ) == true );

	int count = 0;
	bool creating = true;

	b2BodyId bodyIds[BODY_COUNT];
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	b2Polygon square = b2MakeSquare( 0.5f );

	for ( int32_t i = 0; i < 2 * BODY_COUNT + 10; ++i )
	{
		if ( creating )
		{
			if ( count < BODY_COUNT )
			{
				bodyIds[count] = b2CreateBody( worldId, &bodyDef );

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				b2CreatePolygonShape( bodyIds[count], &shapeDef, &square );
				count += 1;
			}
			else
			{
				creating = false;
			}
		}
		else if ( count > 0 )
		{
			b2DestroyBody( bodyIds[count - 1] );
			bodyIds[count - 1] = b2_nullBodyId;
			count -= 1;
		}

		b2World_Step( worldId, 1.0f / 60.0f, 3 );
	}

	b2Counters counters = b2World_GetCounters( worldId );
	ENSURE( counters.bodyCount == 0 );

	b2DestroyWorld( worldId );

	ENSURE( b2World_IsValid( worldId ) == false );

	return 0;
}

static int TestIsValid( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2WorldId worldId = b2CreateWorld( &worldDef );
	ENSURE( b2World_IsValid( worldId ) );

	b2BodyDef bodyDef = b2DefaultBodyDef();

	b2BodyId bodyId1 = b2CreateBody( worldId, &bodyDef );
	ENSURE( b2Body_IsValid( bodyId1 ) == true );

	b2BodyId bodyId2 = b2CreateBody( worldId, &bodyDef );
	ENSURE( b2Body_IsValid( bodyId2 ) == true );

	b2DestroyBody( bodyId1 );
	ENSURE( b2Body_IsValid( bodyId1 ) == false );

	b2DestroyBody( bodyId2 );
	ENSURE( b2Body_IsValid( bodyId2 ) == false );

	b2DestroyWorld( worldId );

	ENSURE( b2World_IsValid( worldId ) == false );
	ENSURE( b2Body_IsValid( bodyId2 ) == false );
	ENSURE( b2Body_IsValid( bodyId1 ) == false );

	return 0;
}

#define WORLD_COUNT ( B2_MAX_WORLDS / 2 )

int TestWorldRecycle( void )
{
	_Static_assert( WORLD_COUNT > 0, "world count" );

	int count = 100;

	b2WorldId worldIds[WORLD_COUNT];

	for ( int i = 0; i < count; ++i )
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		for ( int j = 0; j < WORLD_COUNT; ++j )
		{
			worldIds[j] = b2CreateWorld( &worldDef );
			ENSURE( b2World_IsValid( worldIds[j] ) == true );

			b2BodyDef bodyDef = b2DefaultBodyDef();
			b2CreateBody( worldIds[j], &bodyDef );
		}

		for ( int j = 0; j < WORLD_COUNT; ++j )
		{
			float timeStep = 1.0f / 60.0f;
			int subStepCount = 1;

			for ( int k = 0; k < 10; ++k )
			{
				b2World_Step( worldIds[j], timeStep, subStepCount );
			}
		}

		for ( int j = WORLD_COUNT - 1; j >= 0; --j )
		{
			b2DestroyWorld( worldIds[j] );
			ENSURE( b2World_IsValid( worldIds[j] ) == false );
			worldIds[j] = b2_nullWorldId;
		}
	}

	return 0;
}

static bool CustomFilter( b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* context )
{
	(void)shapeIdA;
	(void)shapeIdB;
	ENSURE( context == NULL );
	return true;
}

static bool PreSolveStatic( b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Vec2 point, b2Vec2 normal, void* context )
{
	(void)shapeIdA;
	(void)shapeIdB;
	(void)point;
	(void)normal;
	ENSURE( context == NULL );
	return false;
}

// This test is here to ensure all API functions link correctly.
int TestWorldCoverage( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();

	b2WorldId worldId = b2CreateWorld( &worldDef );
	ENSURE( b2World_IsValid( worldId ) );

	b2World_EnableSleeping( worldId, true );
	b2World_EnableSleeping( worldId, false );
	bool flag = b2World_IsSleepingEnabled( worldId );
	ENSURE( flag == false );

	b2World_EnableContinuous( worldId, false );
	b2World_EnableContinuous( worldId, true );
	flag = b2World_IsContinuousEnabled( worldId );
	ENSURE( flag == true );

	b2World_SetRestitutionThreshold( worldId, 0.0f );
	b2World_SetRestitutionThreshold( worldId, 2.0f );
	float value = b2World_GetRestitutionThreshold( worldId );
	ENSURE( value == 2.0f );

	b2World_SetHitEventThreshold( worldId, 0.0f );
	b2World_SetHitEventThreshold( worldId, 100.0f );
	value = b2World_GetHitEventThreshold( worldId );
	ENSURE( value == 100.0f );

	b2World_SetCustomFilterCallback( worldId, CustomFilter, NULL );
	b2World_SetPreSolveCallback( worldId, PreSolveStatic, NULL );

	b2Vec2 g = { 1.0f, 2.0f };
	b2World_SetGravity( worldId, g );
	b2Vec2 v = b2World_GetGravity( worldId );
	ENSURE( v.x == g.x );
	ENSURE( v.y == g.y );

	b2ExplosionDef explosionDef = b2DefaultExplosionDef();
	b2World_Explode( worldId, &explosionDef );

	b2World_SetContactTuning( worldId, 10.0f, 2.0f, 4.0f );

	b2World_SetMaximumLinearSpeed( worldId, 10.0f );
	value = b2World_GetMaximumLinearSpeed( worldId );
	ENSURE( value == 10.0f );

	b2World_EnableWarmStarting( worldId, true );
	flag = b2World_IsWarmStartingEnabled( worldId );
	ENSURE( flag == true );

	int count = b2World_GetAwakeBodyCount( worldId );
	ENSURE( count == 0 );

	b2World_SetUserData( worldId, &value );
	void* userData = b2World_GetUserData( worldId );
	ENSURE( userData == &value );

	b2World_Step( worldId, 1.0f, 1 );

	b2DestroyWorld( worldId );

	return 0;
}

static int TestSensor( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	b2WorldId worldId = b2CreateWorld( &worldDef );

	// Wall from x = 1 to x = 2
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position.x = 1.5f;
	bodyDef.position.y = 11.0f;
	b2BodyId wallId = b2CreateBody( worldId, &bodyDef );
	b2Polygon box = b2MakeBox( 0.5f, 10.0f );
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.enableSensorEvents = true;
	b2CreatePolygonShape( wallId, &shapeDef, &box );

	// Bullet fired towards the wall
	bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.isBullet = true;
	bodyDef.gravityScale = 0.0f;
	bodyDef.position = (b2Vec2){ 7.39814f, 4.0f };
	bodyDef.linearVelocity = (b2Vec2){ -20.0f, 0.0f };
	b2BodyId bulletId = b2CreateBody( worldId, &bodyDef );
	shapeDef = b2DefaultShapeDef();
	shapeDef.isSensor = true;
	shapeDef.enableSensorEvents = true;
	b2Circle circle = { { 0.0f, 0.0f }, 0.1f };
	b2CreateCircleShape( bulletId, &shapeDef, &circle );

	int beginCount = 0;
	int endCount = 0;

	while ( true )
	{
		float timeStep = 1.0f / 60.0f;
		int subStepCount = 4;
		b2World_Step( worldId, timeStep, subStepCount );

		b2Vec2 bulletPos = b2Body_GetPosition( bulletId );
		//printf( "Bullet pos: %g %g\n", bulletPos.x, bulletPos.y );

		b2SensorEvents events = b2World_GetSensorEvents( worldId );

		if ( events.beginCount > 0 )
		{
			beginCount += 1;
		}

		if ( events.endCount > 0 )
		{
			endCount += 1;
		}

		if ( bulletPos.x < -1.0f )
		{
			break;
		}
	}

	b2DestroyWorld( worldId );

	ENSURE( beginCount == 1 );
	ENSURE( endCount == 1 );

	return 0;
}

static int TestCollisionMassScale( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = b2Vec2_zero;
	b2WorldId worldId = b2CreateWorld( &worldDef );

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;

	b2BodyId bodyA = b2CreateBody( worldId, &bodyDef );
	b2BodyId bodyB = b2CreateBody( worldId, &bodyDef );

	ENSURE_SMALL( b2Body_GetCollisionMassScale( bodyA ) - 1.0f, 0.0f );
	ENSURE_SMALL( b2Body_GetCollisionMassScale( bodyB ) - 1.0f, 0.0f );

	b2Body_SetCollisionMassScale( bodyA, 0.25f );
	b2Body_SetCollisionMassScale( bodyB, 4.0f );

	ENSURE_SMALL( b2Body_GetCollisionMassScale( bodyA ) - 0.25f, 0.0f );
	ENSURE_SMALL( b2Body_GetCollisionMassScale( bodyB ) - 4.0f, 0.0f );

	b2Circle circle = { { 0.0f, 0.0f }, 1.0f };
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.material.friction = 0.0f;
	shapeDef.material.restitution = 0.0f;
	b2CreateCircleShape( bodyA, &shapeDef, &circle );
	b2CreateCircleShape( bodyB, &shapeDef, &circle );

	b2Body_SetTransform( bodyA, (b2Vec2){ -0.2f, 0.0f }, b2Rot_identity );
	b2Body_SetTransform( bodyB, (b2Vec2){ 0.2f, 0.0f }, b2Rot_identity );

	b2Vec2 initialA = b2Body_GetPosition( bodyA );
	b2Vec2 initialB = b2Body_GetPosition( bodyB );

	for ( int i = 0; i < 20; ++i )
	{
		b2World_Step( worldId, 1.0f / 60.0f, 8 );
	}

	b2Vec2 finalA = b2Body_GetPosition( bodyA );
	b2Vec2 finalB = b2Body_GetPosition( bodyB );

	float moveA = b2Distance( finalA, initialA );
	float moveB = b2Distance( finalB, initialB );

	ENSURE( moveA > 0.0f );
	ENSURE( moveB > 0.0f );
	ENSURE( moveA > moveB * 2.0f );

	b2DestroyWorld( worldId );
	return 0;
}

static int TestCollisionMassScaleLinearRatio( void )
{
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = b2Vec2_zero;
	b2WorldId worldId = b2CreateWorld( &worldDef );

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.material.friction = 0.0f;
	shapeDef.material.restitution = 0.0f;
	b2Circle circle = { { 0.0f, 0.0f }, 1.0f };

	// Case 1: equal scales -> equal separation movement
	b2BodyId equalA = b2CreateBody( worldId, &bodyDef );
	b2BodyId equalB = b2CreateBody( worldId, &bodyDef );
	b2CreateCircleShape( equalA, &shapeDef, &circle );
	b2CreateCircleShape( equalB, &shapeDef, &circle );
	b2Body_SetTransform( equalA, (b2Vec2){ -0.2f, 0.0f }, b2Rot_identity );
	b2Body_SetTransform( equalB, (b2Vec2){ 0.2f, 0.0f }, b2Rot_identity );
	b2Body_SetCollisionMassScale( equalA, 1.0f );
	b2Body_SetCollisionMassScale( equalB, 1.0f );

	// Case 2: 1 vs 3 -> scale 1 body should separate ~3x more
	b2BodyId oneVsThreeA = b2CreateBody( worldId, &bodyDef );
	b2BodyId oneVsThreeB = b2CreateBody( worldId, &bodyDef );
	b2CreateCircleShape( oneVsThreeA, &shapeDef, &circle );
	b2CreateCircleShape( oneVsThreeB, &shapeDef, &circle );
	b2Body_SetTransform( oneVsThreeA, (b2Vec2){ -0.2f, 4.5f }, b2Rot_identity );
	b2Body_SetTransform( oneVsThreeB, (b2Vec2){ 0.2f, 4.5f }, b2Rot_identity );
	b2Body_SetCollisionMassScale( oneVsThreeA, 1.0f );
	b2Body_SetCollisionMassScale( oneVsThreeB, 3.0f );

	// Case 3: 10 vs 3 -> scale 3 body should separate ~10/3 more
	b2BodyId tenVsThreeA = b2CreateBody( worldId, &bodyDef );
	b2BodyId tenVsThreeB = b2CreateBody( worldId, &bodyDef );
	b2CreateCircleShape( tenVsThreeA, &shapeDef, &circle );
	b2CreateCircleShape( tenVsThreeB, &shapeDef, &circle );
	b2Body_SetTransform( tenVsThreeA, (b2Vec2){ -0.2f, 9.0f }, b2Rot_identity );
	b2Body_SetTransform( tenVsThreeB, (b2Vec2){ 0.2f, 9.0f }, b2Rot_identity );
	b2Body_SetCollisionMassScale( tenVsThreeA, 10.0f );
	b2Body_SetCollisionMassScale( tenVsThreeB, 3.0f );

	b2Vec2 equalStartA = b2Body_GetPosition( equalA );
	b2Vec2 equalStartB = b2Body_GetPosition( equalB );
	b2Vec2 oneVsThreeStartA = b2Body_GetPosition( oneVsThreeA );
	b2Vec2 oneVsThreeStartB = b2Body_GetPosition( oneVsThreeB );
	b2Vec2 tenVsThreeStartA = b2Body_GetPosition( tenVsThreeA );
	b2Vec2 tenVsThreeStartB = b2Body_GetPosition( tenVsThreeB );

	for ( int i = 0; i < 20; ++i )
	{
		b2World_Step( worldId, 1.0f / 60.0f, 8 );
	}

	float eqMoveA = b2Distance( equalStartA, b2Body_GetPosition( equalA ) );
	float eqMoveB = b2Distance( equalStartB, b2Body_GetPosition( equalB ) );
	float oneVsThreeMoveA = b2Distance( oneVsThreeStartA, b2Body_GetPosition( oneVsThreeA ) );
	float oneVsThreeMoveB = b2Distance( oneVsThreeStartB, b2Body_GetPosition( oneVsThreeB ) );
	float tenVsThreeMoveA = b2Distance( tenVsThreeStartA, b2Body_GetPosition( tenVsThreeA ) );
	float tenVsThreeMoveB = b2Distance( tenVsThreeStartB, b2Body_GetPosition( tenVsThreeB ) );

	ENSURE( eqMoveA > 0.0f );
	ENSURE( eqMoveB > 0.0f );
	ENSURE( oneVsThreeMoveA > 0.0f );
	ENSURE( oneVsThreeMoveB > 0.0f );
	ENSURE( tenVsThreeMoveA > 0.0f );
	ENSURE( tenVsThreeMoveB > 0.0f );

	float eqRatio = eqMoveA / eqMoveB;
	float oneVsThreeRatio = oneVsThreeMoveA / oneVsThreeMoveB;
	float tenVsThreeRatio = tenVsThreeMoveB / tenVsThreeMoveA;

	ENSURE( 0.8f < eqRatio && eqRatio < 1.2f );
	ENSURE( 2.5f < oneVsThreeRatio && oneVsThreeRatio < 3.5f );
	ENSURE( 2.8f < tenVsThreeRatio && tenVsThreeRatio < 3.9f );

	b2DestroyWorld( worldId );
	return 0;
}

int WorldTest( void )
{
	RUN_SUBTEST( HelloWorld );
	RUN_SUBTEST( EmptyWorld );
	RUN_SUBTEST( DestroyAllBodiesWorld );
	RUN_SUBTEST( TestIsValid );
	RUN_SUBTEST( TestWorldRecycle );
	RUN_SUBTEST( TestWorldCoverage );
	RUN_SUBTEST( TestSensor );
	RUN_SUBTEST( TestCollisionMassScale );
	RUN_SUBTEST( TestCollisionMassScaleLinearRatio );

	return 0;
}
