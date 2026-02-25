using System;
using HRealEngine;

namespace ExampleProject
{
    public class Enemy_Example : Entity
    {
        void BeginPlay()
        {
            
        }
        void OnDestroy()
        {
            
        }
        void Tick(float ts)
        {
            
        }
        void OnCollisionEnter(ulong otherEntityID)
        {
            
        }
        void OnCollisionExit(ulong otherEntityID)
        {
            
        }

        public override void OnEntityPerceived(ulong entityID, int perceptionMethod, Vector3 position)
        {
            Console.WriteLine($"Enemy perceived entity {entityID} at position {position} with perception method {perceptionMethod}");
        }

        public override void OnEntityLost(ulong entityID, Vector3 lastKnownPosition)
        {
            Console.WriteLine($"Enemy lost entity {entityID} last known position was {lastKnownPosition}");
        }

        public override void OnEntityForgotten(ulong entityID)
        {
            Console.WriteLine($"Enemy forgot entity {entityID}");
        }
    }
}