<h1>Introduction</h1>
For my research project I decided to find the advantages and disadvantages of the 2 decision making structures we have seen in class: finite state machine and behavior tree.
Then I used this knowledge to combine the 2 for a more optimal decision making.
I also upgraded the behavior tree by adding decorators and queries.
<h1>Advantages and disadvantages of a finite state machine</h1>
<h2>Advantages</h2>
Finite state machines are easy to implement and easy to understand.
<h2>Disadvantages</h2>
Finite state machines don’t scale well. There are n^2 possible transitions for each state. When a finite state machine becomes too big it becomes messy and unclear. Each action needs to know what to do next. 
They also have the GOTO anology problem. A GOTO statement is a "jump" between lines of code. In code with a lot of GOTO statements it is hard to expand, understand and debug the code. Finite state machines are full of them (transitions).
<h1>Advantages and disadvantages of a behavior tree</h1>
<h2>Advantages</h2>
Behavior trees are modular which means there is a weak dependence between subtrees. They don't need to know what action to do next like in the finite state machine. The only thing it needs to know is if the action failed or succeeded. It makes it easier to expand the code for example. They use the function call analogy instead of the GOTO analogy.
Which means they use function calls instead of GOTO statements.
<h2>Disadvantages</h2>
They are harder to implement and understand than finite state machines. 

<h1>Conclusion</h1>
Finites state machines are useful for smaller and easier problems while behavior trees are more useful for bigger and more complex problems.

<h1>Project</h1>
For my project I expanded on the agario game we had seen in class.

I started with a behavior tree as the main decision making and made a finite state machine for the decision making when the agent is close to an enemy.

![98df548c09a777116e2990b790691971](https://user-images.githubusercontent.com/76271773/105632660-9c735500-5e54-11eb-8b01-91f1b5a03f88.png)

I am using the same blackboard for both decision making structures and I have a state/action where I can switch decision making structure.

![491989d010110690d961bcb804deff62](https://user-images.githubusercontent.com/76271773/105633216-97fc6b80-5e57-11eb-854f-2e6230310580.png)

In the finites state machine the agent evades enemies that are bigger and pursuits enemies that are smaller.
It switches back to the behavior tree when it is done.

![2079f7014fc16b6ba7a3c744f49e0400](https://user-images.githubusercontent.com/76271773/105642792-4f12da00-5e8c-11eb-86a0-1fec85d5d868.png)

The finite state machine also saves its start state now and has a member function reset that sets the state back to the start state.
This member function can be called before switching to the finite state machine to make it always start from its start state.

I used this technique in the zombie project too.
The main decision structure is a behavior tree and it switches to a finite state machine for smaller tasks like searching and leaving houses and avoiding enemies.

<h1>Decorators and queries</h1>
These are included in the research project but I only use them in the zombie game.
<h2>Decorators</h2>
The decorator works like the blackboard based condition decorator from Unreal Engine.
It takes a key from the blackboard which has to be a boolean and a second parameter "isSet" which determines if that key value has to be true or false.
If the key value is equal to isSet, it return success, otherwise it returns failure.

![85a6270b844ab9a0d7c59ea30c044f4e](https://user-images.githubusercontent.com/76271773/105633912-a056a580-5e5b-11eb-9427-176f93f2de64.png)

<h2>Queries</h2>
A query is a function that stores its result in a blackboard key.
The key name and function has to be passed to the query and the type of the result has to be specified.
If the function didn't fail it returns success, otherwise it returns failure.
A query can be used to get certain information in a blackboard key.
It is also useful when you need certain data in multiple conditionals so you don't need to write the same code multiple times.
For example if you need the closest enemy for a conditional you can call a query before the conditional gets executed.

![2142283e906a255d1bfa826419461156](https://user-images.githubusercontent.com/76271773/105634273-7e5e2280-5e5d-11eb-89e8-bdddd1a9d66d.png)
