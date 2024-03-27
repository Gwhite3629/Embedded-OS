The queue system is how programs communicate with eachother within the node network. This is intertwined with the properties outlined in the ***node properties*** folder.

There are 3 levels of priority within the queue system.
1. Lazy Trigger
2. Demand
3. Lazy Low Priority

These levels all have their own queue with independent properties.

# **Lazy Trigger**

The lazy trigger queue (LTQ) is responsible for collecting events with the property by the same name. The process consists of sending a lazy request as normal but with a trigger condition. The trigger condition from the queue perspective is just a generic trigger request with an ID associated with it. The ID is necessary because multiple nodes can send multiple trigger requests to the LTQ. The node which sent the trigger request is responsible for sending the trigger. The cause of the trigger is internal to that node. Upon recieving the trigger, the request is promoted to the demand queue and is placed on top of the queue. This is to indicate the immediate nature of lazy trigger requests. These type of events could be used for a more time-critical situation. Any time critical type of requests should be prepared ahead of time so they can be triggered appropriately. Trigger type events should not be sent and then immediately triggered. This would be a demand type of situation. The intent of the LTQ is to prepare events ahead of time.

# **Demand**

The demand queue (DQ) is responsible for collecting events with the demand property. The demand property is considered the default high priority type of access. This is how access is handled for events that are needed but are not of a time-critical nature.

# **Lazy Low priority**

The lazy low priority queue (LPQ) is for events of a very non-critical nature. These requests are executed when the DQ is empty. There is no preemption for the events, so if a LPQ event is taken and a TQ is triggered, the TQ will have to wait.