PROGRAM 5

QUESTION: Implement Singly, Doubly, and Circular Linked Lists with insertion, deletion, and traversal.

A. Singly Linked List

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.next = None

class SinglyLinkedList:
    def __init__(self):
        self.head = None

    def insert(self, data):
        node = Node(data)
        if self.head is None:
            self.head = node
            return
        temp = self.head
        while temp.next:
            temp = temp.next
        temp.next = node

    def delete(self, data):
        temp = self.head
        prev = None
        while temp and temp.data != data:
            prev = temp
            temp = temp.next
        if temp:
            if prev:
                prev.next = temp.next
            else:
                self.head = temp.next

    def display(self):
        temp = self.head
        while temp:
            print(temp.data, end=" ")
            temp = temp.next
        print()

s = SinglyLinkedList()
s.insert(10)
s.insert(20)
s.insert(30)
print("Singly linked list:")
s.display()
s.delete(20)
print("After deletion:")
s.display()

OUTPUT:
Singly linked list:
10 20 30
After deletion:
10 30


B. Doubly Linked List

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.prev = None
        self.next = None

class DoublyLinkedList:
    def __init__(self):
        self.head = None

    def insert(self, data):
        node = Node(data)
        if self.head is None:
            self.head = node
            return
        temp = self.head
        while temp.next:
            temp = temp.next
        temp.next = node
        node.prev = temp

    def delete(self, data):
        temp = self.head
        while temp and temp.data != data:
            temp = temp.next
        if temp is None:
            return
        if temp.prev:
            temp.prev.next = temp.next
        else:
            self.head = temp.next
        if temp.next:
            temp.next.prev = temp.prev

    def display(self):
        temp = self.head
        while temp:
            print(temp.data, end=" ")
            temp = temp.next
        print()

d = DoublyLinkedList()
d.insert(10)
d.insert(20)
d.insert(30)
print("Doubly linked list:")
d.display()
d.delete(20)
print("After deletion:")
d.display()

OUTPUT:
Doubly linked list:
10 20 30
After deletion:
10 30


C. Circular Linked List

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.next = None

class CircularLinkedList:
    def __init__(self):
        self.head = None

    def insert(self, data):
        node = Node(data)
        if self.head is None:
            self.head = node
            node.next = self.head
            return
        temp = self.head
        while temp.next != self.head:
            temp = temp.next
        temp.next = node
        node.next = self.head

    def delete(self, data):
        if self.head is None:
            return
        curr = self.head
        prev = None
        while True:
            if curr.data == data:
                if prev is None:
                    if curr.next == self.head:
                        self.head = None
                    else:
                        tail = self.head
                        while tail.next != self.head:
                            tail = tail.next
                        self.head = curr.next
                        tail.next = self.head
                else:
                    prev.next = curr.next
                return
            prev = curr
            curr = curr.next
            if curr == self.head:
                return

    def display(self):
        if self.head is None:
            print()
            return
        temp = self.head
        while True:
            print(temp.data, end=" ")
            temp = temp.next
            if temp == self.head:
                break
        print()

c = CircularLinkedList()
c.insert(10)
c.insert(20)
c.insert(30)
print("Circular linked list:")
c.display()
c.delete(20)
print("After deletion:")
c.display()

OUTPUT:
Circular linked list:
10 20 30
After deletion:
10 30


PROGRAM 6

QUESTION: Implement Stack and Queue using lists, infix-to-postfix conversion, and queue simulation.

A. Stack Using List

CODE:

stack = []
stack.append(10)
stack.append(20)
stack.append(30)
print("Stack:", stack)
print("Popped element:", stack.pop())
print("Stack after pop:", stack)

OUTPUT:
Stack: [10, 20, 30]
Popped element: 30
Stack after pop: [10, 20]


B. Queue Using List

CODE:

queue = []
queue.append(10)
queue.append(20)
queue.append(30)
print("Queue:", queue)
print("Dequeued element:", queue.pop(0))
print("Queue after dequeue:", queue)

OUTPUT:
Queue: [10, 20, 30]
Dequeued element: 10
Queue after dequeue: [20, 30]


C. Infix-to-Postfix Conversion

CODE:

def precedence(op):
    if op in ("+", "-"):
        return 1
    if op in ("*", "/"):
        return 2
    return 0

def infix_to_postfix(expression):
    result = []
    stack = []
    for ch in expression:
        if ch.isalnum():
            result.append(ch)
        elif ch == "(":
            stack.append(ch)
        elif ch == ")":
            while stack and stack[-1] != "(":
                result.append(stack.pop())
            if stack:
                stack.pop()
        else:
            while stack and stack[-1] != "(" and precedence(stack[-1]) >= precedence(ch):
                result.append(stack.pop())
            stack.append(ch)
    while stack:
        result.append(stack.pop())
    return "".join(result)

expression = "A+B*C"
print("Infix expression:", expression)
print("Postfix expression:", infix_to_postfix(expression))

OUTPUT:
Infix expression: A+B*C
Postfix expression: ABC*+


D. Queue Simulation

CODE:

from collections import deque

queue = deque()
queue.append("Customer 1")
queue.append("Customer 2")
queue.append("Customer 3")
print("Waiting queue:", list(queue))
served = queue.popleft()
print("Served:", served)
print("Remaining queue:", list(queue))

OUTPUT:
Waiting queue: ['Customer 1', 'Customer 2', 'Customer 3']
Served: Customer 1
Remaining queue: ['Customer 2', 'Customer 3']


PROGRAM 7

QUESTION: Implement a Binary Tree and perform Inorder, Preorder, and Postorder traversals.

A. Inorder Traversal

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def inorder(node):
    if node:
        inorder(node.left)
        print(node.data, end=" ")
        inorder(node.right)

node = Node(1)
node.left = Node(2)
node.right = Node(3)
node.left.left = Node(4)
node.left.right = Node(5)

print("Inorder traversal:", end=" ")
inorder(node)
print()

OUTPUT:
Inorder traversal: 4 2 5 1 3


B. Preorder Traversal

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def preorder(node):
    if node:
        print(node.data, end=" ")
        preorder(node.left)
        preorder(node.right)

node = Node(1)
node.left = Node(2)
node.right = Node(3)
node.left.left = Node(4)
node.left.right = Node(5)

print("Preorder traversal:", end=" ")
preorder(node)
print()

OUTPUT:
Preorder traversal: 1 2 4 5 3


C. Postorder Traversal

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def postorder(node):
    if node:
        postorder(node.left)
        postorder(node.right)
        print(node.data, end=" ")

node = Node(1)
node.left = Node(2)
node.right = Node(3)
node.left.left = Node(4)
node.left.right = Node(5)

print("Postorder traversal:", end=" ")
postorder(node)
print()

OUTPUT:
Postorder traversal: 4 5 2 3 1


PROGRAM 8

QUESTION: Implement Binary Search Tree operations and Graph Traversal Algorithms (BFS, DFS). Analyze their efficiency.

A. Binary Search Tree Insertion and Traversal

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def insert(node, data):
    if node is None:
        return Node(data)
    if data < node.data:
        node.left = insert(node.left, data)
    elif data > node.data:
        node.right = insert(node.right, data)
    return node

def inorder(node):
    if node:
        inorder(node.left)
        print(node.data, end=" ")
        inorder(node.right)

node = None
for value in [50, 30, 70, 20, 40, 60, 80]:
    node = insert(node, value)

print("BST inorder traversal:", end=" ")
inorder(node)
print()

OUTPUT:
BST inorder traversal: 20 30 40 50 60 70 80


B. Binary Search Tree Searching

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def insert(node, data):
    if node is None:
        return Node(data)
    if data < node.data:
        node.left = insert(node.left, data)
    elif data > node.data:
        node.right = insert(node.right, data)
    return node

def search(node, key):
    if node is None or node.data == key:
        return node
    if key < node.data:
        return search(node.left, key)
    return search(node.right, key)

node = None
for value in [50, 30, 70, 20, 40, 60, 80]:
    node = insert(node, value)

key = 40
if search(node, key):
    print("Element found:", key)
else:
    print("Element not found")

OUTPUT:
Element found: 40


C. Binary Search Tree Deletion

CODE:

class Node:
    def __init__(self, data):
        self.data = data
        self.left = None
        self.right = None

def insert(node, data):
    if node is None:
        return Node(data)
    if data < node.data:
        node.left = insert(node.left, data)
    elif data > node.data:
        node.right = insert(node.right, data)
    return node

def delete(node, key):
    if node is None:
        return None
    if key < node.data:
        node.left = delete(node.left, key)
    elif key > node.data:
        node.right = delete(node.right, key)
    else:
        if node.left is None:
            return node.right
        if node.right is None:
            return node.left
        temp = node.right
        while temp.left:
            temp = temp.left
        node.data = temp.data
        node.right = delete(node.right, temp.data)
    return node

def inorder(node):
    if node:
        inorder(node.left)
        print(node.data, end=" ")
        inorder(node.right)

node = None
for value in [50, 30, 70, 20, 40, 60, 80]:
    node = insert(node, value)

node = delete(node, 30)
print("BST after deletion:", end=" ")
inorder(node)
print()

OUTPUT:
BST after deletion: 20 40 50 60 70 80


D. Breadth-First Search (BFS)

CODE:

from collections import deque

graph = {
    'A': ['B', 'C'],
    'B': ['A', 'D', 'E'],
    'C': ['A', 'F'],
    'D': ['B'],
    'E': ['B'],
    'F': ['C']
}

def bfs(graph, start):
    visited = {start}
    queue = deque([start])
    while queue:
        node = queue.popleft()
        print(node, end=" ")
        for neighbor in graph[node]:
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)

print("BFS traversal:", end=" ")
bfs(graph, 'A')
print()

OUTPUT:
BFS traversal: A B C D E F


E. Depth-First Search (DFS)

CODE:

graph = {
    'A': ['B', 'C'],
    'B': ['A', 'D', 'E'],
    'C': ['A', 'F'],
    'D': ['B'],
    'E': ['B'],
    'F': ['C']
}

def dfs(graph, start, visited=None):
    if visited is None:
        visited = set()
    visited.add(start)
    print(start, end=" ")
    for neighbor in graph[start]:
        if neighbor not in visited:
            dfs(graph, neighbor, visited)

print("DFS traversal:", end=" ")
dfs(graph, 'A')
print()

OUTPUT:
DFS traversal: A B D E C F


F. Efficiency Analysis

CODE:

print("BST average search time: O(log n)")
print("BST worst-case search time: O(n)")
print("BFS time complexity: O(V + E)")
print("DFS time complexity: O(V + E)")

OUTPUT:
BST average search time: O(log n)
BST worst-case search time: O(n)
BFS time complexity: O(V + E)
DFS time complexity: O(V + E)


EFFICIENCY TABLE

| Algorithm | Best / Average Case | Worst Case |
|---|---|---|
| BST Search | O(log n) | O(n) |
| BST Insertion | O(log n) | O(n) |
| BST Deletion | O(log n) | O(n) |
| BFS | O(V + E) | O(V + E) |
| DFS | O(V + E) | O(V + E) |

NOTE: V represents the number of vertices, and E represents the number of edges. An unbalanced Binary Search Tree can take O(n) time in the worst case.
