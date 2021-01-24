<h1>Introduction</h1>
For my research project I decided to find the advantages and disadvantages of the 2 decision making structures we have seen in class: finite state machine and behavior tree.
Then I used this knowledge to combine the 2 for a more optimal decision making.
I also upgraded the behavior tree by adding decorators and queries.
<h1>Advantages and disadvantages of a finite state machine</h1>
<h2>Advantages</h2>
Finite state machines are easy to implement and easy to understand.
<h2>Disadvantages</h2>
Finite state machines don’t scale well. There are n^2 possible transitions for each state. When a finite state machine becomes too big it becomes messy and unclear. Each action needs to know what to do next. 
They also have the GOTO anology problem. In code with a lot of GOTO statements it is hard to expand, understand and debug the code. Finite state machines are full of them (transitions).
<h1>Advantages and disadvantages of a behavior tree</h1>
<h2>Advantages</h2>
Behavior trees are modular which means there is a weak dependence between subtrees. It makes it easier to expand the code for example. They use the function call analogy instead of the GOTO analogy. 
<h2>Disadvantages</h2>
They are harder to implement and understand than finite state machines. 

<h1>Conclusion</h1>
Finites state machines are useful for smaller and easier problems while behavior trees are more useful for bigger and more complex problems.

<h1>Project</h1>
For my project I expanded on the agario game we had seen in class.

