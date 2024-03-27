All programs in the node structure have certain properties associated with them. There are 4 categories and 2 options in each category. Every process has 4 properties.

The categories are as follows:
1. Access: Shared / Private
2. Urgency: Demand / Lazy
3. Safety: Computational / Interactive
4. Distribution: Serial / Parallel

All nodes contain an internal set of priority queues which dictate how access is handled and how requests are handled. These queues are written to from some external protocol like through ethernet or USB. This is opaque to the programmer and the data is inserted into queues on an interrupt.

# **Access**

The access property describes how the program / node(s) interact with the other nodes in the system.

## Shared

A shared program means that it can be accessed by any other shared program in the system. Data declared as 'shared' within the program can be accessed through the queue interface.

## Private

A private program means that it can only be accessed by internal resources or other programs in its group. Private nodes can also be accessed by nodes with specified access. This access can only be granted by the private node itself, there is no method of access request.

# **Urgency**

The urgency property determines how a programs requests are handled by the queue system. There are 3 types of queue, 2 of which are lazy.

## Demand

The demand queue is the queue that feeds directly into the programs request handler. The demand queue has the second level of priority of the 3 queues. Programs who have the demand property feed to this queue.

## Lazy

The lazy property has 2 subtypes. There is a trigger queue which has the highest priority and the low priority lazy queue. The trigger queue has a special property that when it is initially inserted to its queue it remains there until it is triggered. At that point it is promoted to the highest priority and inserted at the top of the demand queue. The low priority queue is only promoted to demand when the demand queue is empty.

# **Safety**

The safety property determines if the program contains stateful side effects or not. This qualifier is more relevant for specific functions than programs, but a program containing computational functions should be marked as such.

## Computationel

Computational programs contain a strict structure that allows their computational sections to be contained in a monad structure. This essentially allows for static checking of the computational function and for strict input and output from the computational unit in a stateless manner. The implementation of this is described in the ***monad*** folder.

## Interactive

Interactive programs essentially have no need for a computational structure. This is how most regular C programs exist. Interactive programs specifically deal with lots of input and output, likely via the queue system.

# **Distribution**

The distribution property determines if the program is part of a task group or not.

## Serial

The serial property means the program is not part of a task group and operates on an independent node.

## Parallel

The parallel property indicates that the node is part of a task group and shares resources amongst the other nodes in the group. There is a sort of implicit sharing amongst members of a task group that supercedes the **access** property. The members of a task group do not have to be homogenous and there can be a mix of properties within the group.