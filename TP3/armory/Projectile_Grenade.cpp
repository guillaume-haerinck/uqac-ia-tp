#include "Projectile_Grenade.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/cgdi.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "../constants.h"
#include "2d/WallIntersectionTests.h"
#include "../Raven_Map.h"

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"


//-------------------------- ctor ---------------------------------------------
//-----------------------------------------------------------------------------
Grenade_Projectile::Grenade_Projectile(Raven_Bot* shooter, Vector2D target) :

	Raven_Projectile(target,
		shooter->GetWorld(),
		shooter->ID(),
		shooter->Pos(),
		shooter->Facing(),
		script->GetInt("Grenade_Damage"),
		script->GetDouble("Grenade_Scale"),
		script->GetDouble("Grenade_MaxSpeed"),
		script->GetDouble("Grenade_Mass"),
		script->GetDouble("Grenade_MaxForce")),

	exploded(false),
	explosionTime(Clock->GetCurrentTime() + script->GetDouble("Grenade_TimeBeforeExplosion")),
	m_dCurrentBlastRadius(0.0),
	m_dBlastRadius(script->GetDouble("Grenade_BlastRadius"))
{
	assert(target != Vector2D());
}


//------------------------------ Update ---------------------------------------
//-----------------------------------------------------------------------------
void Grenade_Projectile::Update()
{
	if (!m_bImpacted)
	{
		m_vVelocity = MaxSpeed() * Heading();

		//make sure vehicle does not exceed maximum velocity
		m_vVelocity.Truncate(m_dMaxSpeed);

		//update the position
		m_vPosition += m_vVelocity;

		TestForImpact();
	}

	if (isExploded())
	{
		if (!exploded) {
			InflictDamageOnBotsWithinBlastRadius();

			exploded = true;
		}
		
		m_dCurrentBlastRadius += script->GetDouble("Rocket_ExplosionDecayRate");

		//when the rendered blast circle becomes equal in size to the blast radius
		//the rocket can be removed from the game
		if (m_dCurrentBlastRadius > m_dBlastRadius)
		{
			m_bDead = true;
		}
	}
}

void Grenade_Projectile::TestForImpact()
{
	//if the projectile has reached the target position or it hits an entity
	//or wall it should explode/inflict damage/whatever and then mark itself
	//as dead


	//test to see if the line segment connecting the rocket's current position
	//and previous position intersects with any bots.
	Raven_Bot* hit = GetClosestIntersectingBot(m_vPosition - m_vVelocity, m_vPosition);

	//if hit
	if (hit)
	{
		m_bImpacted = true;
		int damageOnHit = 0;

		//send a message to the bot to let it know it's been hit, and who the
		//shot came from
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
			m_iShooterID,
			hit->ID(),
			Msg_TakeThatMF,
			(void *)&damageOnHit);
	}

	//test for impact with a wall
	double dist;
	if (FindClosestPointOfIntersectionWithWalls(m_vPosition - m_vVelocity,
		m_vPosition,
		dist,
		m_vImpactPoint,
		m_pWorld->GetMap()->GetWalls()))
	{
		m_bImpacted = true;

		m_vPosition = m_vImpactPoint;
	}

	const double tolerance = 5.0;
	if (Vec2DDistanceSq(Pos(), m_vTarget) < tolerance*tolerance)
	{
		m_bImpacted = true;
	}
}

//--------------- InflictDamageOnBotsWithinBlastRadius ------------------------
//
//  If the rocket has impacted we test all bots to see if they are within the 
//  blast radius and reduce their health accordingly
//-----------------------------------------------------------------------------
void Grenade_Projectile::InflictDamageOnBotsWithinBlastRadius()
{
	std::list<Raven_Bot*>::const_iterator curBot = m_pWorld->GetAllBots().begin();

	for (curBot; curBot != m_pWorld->GetAllBots().end(); ++curBot)
	{
		if (Vec2DDistance(Pos(), (*curBot)->Pos()) < m_dBlastRadius + (*curBot)->BRadius())
		{
			//send a message to the bot to let it know it's been hit, and who the
			//shot came from
			Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
				m_iShooterID,
				(*curBot)->ID(),
				Msg_TakeThatMF,
				(void*)&m_iDamageInflicted);

		}
	}
}


//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
void Grenade_Projectile::Render()
{

	gdi->DarkGreenPen();
	gdi->DarkGreenBrush();
	gdi->Circle(Pos(), 4);

	if (isExploded())
	{
		gdi->HollowBrush();
		gdi->Circle(Pos(), m_dCurrentBlastRadius);
	}
}