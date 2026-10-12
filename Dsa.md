PROGRAM 1

QUESTION: Implement operations on Lists, Tuples, Sets, and Dictionaries including insertion, deletion, slicing, and comprehensions.

A. List Operations

CODE:
my_list = [10, 20, 30, 40, 50]
print("Original list:", my_list)

my_list.append(60)
my_list.insert(2, 25)
print("After insertion:", my_list)

my_list.remove(40)
removed = my_list.pop()
print("After deletion:", my_list)
print("Popped element:", removed)

print("Slice [1:4]:", my_list[1:4])
print("Reversed list:", my_list[::-1])

squares = [x**2 for x in range(1, 6)]
print("List comprehension (squares):", squares)

OUTPUT:
Original list: [10, 20, 30, 40, 50]
After insertion: [10, 20, 25, 30, 40, 50, 60]
After deletion: [10, 20, 25, 30, 50]
Popped element: 60
Slice [1:4]: [20, 25, 30]
Reversed list: [50, 30, 25, 20, 10]
List comprehension (squares): [1, 4, 9, 16, 25]


B. Tuple Operations

CODE:
my_tuple = (10, 20, 30, 40, 50)
print("Original tuple:", my_tuple)

print("First element:", my_tuple[0])
print("Slice [1:4]:", my_tuple[1:4])
print("Reversed tuple:", my_tuple[::-1])

# Tuples are immutable, so insertion/deletion creates a new tuple
my_tuple = my_tuple + (60,)
print("After insertion:", my_tuple)

my_list = list(my_tuple)
my_list.remove(30)
my_tuple = tuple(my_list)
print("After deletion:", my_tuple)

doubles = tuple(x * 2 for x in my_tuple)
print("Tuple comprehension (doubles):", doubles)

OUTPUT:
Original tuple: (10, 20, 30, 40, 50)
First element: 10
Slice [1:4]: (20, 30, 40)
Reversed tuple: (50, 40, 30, 20, 10)
After insertion: (10, 20, 30, 40, 50, 60)
After deletion: (10, 20, 40, 50, 60)
Tuple comprehension (doubles): (20, 40, 80, 100, 120)


C. Set Operations

CODE:
my_set = {1, 2, 3, 4, 5}
print("Original set:", my_set)

my_set.add(6)
print("After insertion:", my_set)

my_set.remove(3)
my_set.discard(10)
print("After deletion:", my_set)

# Sets are unordered, so slicing is done after converting to a sorted list
print("Slice of sorted set [1:4]:", sorted(my_set)[1:4])

even = {x for x in range(1, 11) if x % 2 == 0}
print("Set comprehension (even numbers):", even)

a = {1, 2, 3}
b = {3, 4, 5}
print("Union:", a | b)
print("Intersection:", a & b)
print("Difference:", a - b)

OUTPUT:
Original set: {1, 2, 3, 4, 5}
After insertion: {1, 2, 3, 4, 5, 6}
After deletion: {1, 2, 4, 5, 6}
Slice of sorted set [1:4]: [2, 4, 5]
Set comprehension (even numbers): {2, 4, 6, 8, 10}
Union: {1, 2, 3, 4, 5}
Intersection: {3}
Difference: {1, 2}


D. Dictionary Operations

CODE:
student = {"name": "Deonna", "age": 19, "branch": "AIML"}
print("Original dictionary:", student)

student["year"] = 2
print("After insertion:", student)

del student["age"]
print("After deletion:", student)

keys = list(student.keys())
print("Slice of keys [0:2]:", keys[0:2])

cubes = {x: x**3 for x in range(1, 6)}
print("Dictionary comprehension (cubes):", cubes)

OUTPUT:
Original dictionary: {'name': 'Deonna', 'age': 19, 'branch': 'AIML'}
After insertion: {'name': 'Deonna', 'age': 19, 'branch': 'AIML', 'year': 2}
After deletion: {'name': 'Deonna', 'branch': 'AIML', 'year': 2}
Slice of keys [0:2]: ['name', 'branch']
Dictionary comprehension (cubes): {1: 1, 2: 8, 3: 27, 4: 64, 5: 125}


==================================================

PROGRAM 2

QUESTION: Develop a Python program to perform string manipulation using slicing and comprehensions.

A. String Slicing

CODE:
text = "Python Programming"
print("Original string:", text)

print("First 6 characters:", text[0:6])
print("Last 11 characters:", text[-11:])
print("Reversed string:", text[::-1])
print("Every 2nd character:", text[::2])

OUTPUT:
Original string: Python Programming
First 6 characters: Python
Last 11 characters: Programming
Reversed string: gnimmargorP nohtyP
Every 2nd character: Pto rgamn


B. String Comprehensions

CODE:
text = "Python Programming"

vowels = "aeiouAEIOU"
only_vowels = [ch for ch in text if ch in vowels]
print("Vowels:", only_vowels)
print("Vowel count:", len(only_vowels))

no_spaces = "".join([ch for ch in text if ch != " "])
print("String without spaces:", no_spaces)

upper_chars = "".join([ch.upper() for ch in text])
print("Uppercase:", upper_chars)

words = text.split()
print("Words:", words)
print("Word lengths:", [len(w) for w in words])

OUTPUT:
Vowels: ['o', 'o', 'a', 'i']
Vowel count: 4
String without spaces: PythonProgramming
Uppercase: PYTHON PROGRAMMING
Words: ['Python', 'Programming']
Word lengths: [6, 11]


C. Palindrome Check and Word Reversal

CODE:
text = "madam"

if text == text[::-1]:
    print(text, "is a palindrome")
else:
    print(text, "is not a palindrome")

sentence = "I love Python programming"
reversed_words = " ".join(sentence.split()[::-1])
print("Reversed word order:", reversed_words)

OUTPUT:
madam is a palindrome
Reversed word order: programming Python love I


==================================================

PROGRAM 3

QUESTION: Implement 1D and 2D arrays and perform operations such as traversal, insertion, and deletion.

A. 1D Array

CODE:
arr = [10, 20, 30, 40, 50]
print("1D array:", arr)

print("Traversal:", end=" ")
for item in arr:
    print(item, end=" ")
print()

arr.insert(2, 25)
print("After insertion at index 2:", arr)

arr.pop(4)
print("After deletion at index 4:", arr)

OUTPUT:
1D array: [10, 20, 30, 40, 50]
Traversal: 10 20 30 40 50
After insertion at index 2: [10, 20, 25, 30, 40, 50]
After deletion at index 4: [10, 20, 25, 30, 50]


B. 2D Array

CODE:
matrix = [[1, 2, 3],
          [4, 5, 6],
          [7, 8, 9]]

print("2D array traversal:")
for row in matrix:
    for element in row:
        print(element, end=" ")
    print()

matrix.insert(1, [10, 11, 12])
print("After row insertion at index 1:")
for row in matrix:
    print(row)

matrix.pop(2)
print("After row deletion at index 2:")
for row in matrix:
    print(row)

matrix[0].insert(1, 99)
print("After inserting 99 in first row:")
print(matrix[0])

OUTPUT:
2D array traversal:
1 2 3
4 5 6
7 8 9
After row insertion at index 1:
[1, 2, 3]
[10, 11, 12]
[4, 5, 6]
[7, 8, 9]
After row deletion at index 2:
[1, 2, 3]
[10, 11, 12]
[7, 8, 9]
After inserting 99 in first row:
[1, 99, 2, 3]


==================================================

PROGRAM 4

QUESTION: Implement searching algorithms (Linear Search, Binary Search) and sorting algorithms (Bubble Sort, Selection Sort, Insertion Sort, Merge Sort, Quick Sort). Compare their performance.

A. Linear Search

CODE:
from array import array

arr = array('i', [10, 20, 30, 40, 50])
key = int(input("Enter the element to search: "))
found = 0

for i in range(len(arr)):
    if arr[i] == key:
        found = 1
        print("Element found at index:", i)
        break

if found == 0:
    print("Element not found")
OUTPUT
Enter the element to search: 30
Element found at index: 2

B . Binary search
CODE
from array import array

arr = array('i', [10, 20, 30, 40, 50])
key = int(input("Enter the element to search: "))
low = 0
high = len(arr) - 1
found = 0

while low <= high:
    mid = (low + high) // 2
    if arr[mid] == key:
        found = 1
        print("Element found at index:", mid)
        break
    elif arr[mid] < key:
        low = mid + 1
    else:
        high = mid - 1

if found == 0:
    print("Element not found")

OUTPUT
Enter the element to search: 40
Element found at index: 3
C. Bubble Sort

CODE:
import array

arr = array.array('i', [50, 20, 40, 10, 30])
print("Original array:", list(arr))

n = len(arr)

for i in range(n):
    for j in range(n - 1):
        if arr[j] > arr[j + 1]:
            arr[j], arr[j + 1] = arr[j + 1], arr[j]

print("Sorted array:", list(arr))

OUTPUT:
Original array: [50, 20, 40, 10, 30]
Sorted array: [10, 20, 30, 40, 50]

D. Selection Sort

CODE:
def selection_sort(arr):
    n = len(arr)
    for i in range(n - 1):
        min_index = i
        for j in range(i + 1, n):
            if arr[j] < arr[min_index]:
                min_index = j
        arr[i], arr[min_index] = arr[min_index], arr[i]

arr = [64, 25, 12, 22, 11]
print("Original array:", arr)

selection_sort(arr)
print("Sorted array:", arr)

OUTPUT:
Original array: [64, 25, 12, 22, 11]
Sorted array: [11, 12, 22, 25, 64]


E. Insertion Sort

CODE:
def insertion_sort(arr):
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and arr[j] > key:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key

arr = [64, 25, 12, 22, 11]
print("Original array:", arr)

insertion_sort(arr)
print("Sorted array:", arr)

OUTPUT:
Original array: [64, 25, 12, 22, 11]
Sorted array: [11, 12, 22, 25, 64]


F. Merge Sort

CODE:
 def merge_sort(arr):
    if len(arr) <= 1:
        return arr

    mid = len(arr) // 2
    left = merge_sort(arr[:mid])
    right = merge_sort(arr[mid:])

    return merge(left, right)

def merge(left, right):
    result = []
    i = 0
    j = 0

    while i < len(left) and j < len(right):
        if left[i] < right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1

    result.extend(left[i:])
    result.extend(right[j:])

    return result

arr = [38, 12, 27, 43, 9, 31, 18, 25]
print("Original array:", arr)
print("Sorted array:", merge_sort(arr))

OUTPUT
Original array: [38, 12, 27, 43, 9, 31, 18, 25]
Sorted array: [9, 12, 18, 25, 27, 31, 38, 43]


G. Quick Sort

CODE:
def partition(arr, low, high):
    pivot = arr[high]
    i = low - 1

    for j in range(low, high):
        if arr[j] <= pivot:
            i += 1
            arr[i], arr[j] = arr[j], arr[i]

    arr[i + 1], arr[high] = arr[high], arr[i + 1]
    return i + 1

def quick_sort(arr, low, high):
    if low < high:
        p = partition(arr, low, high)
        quick_sort(arr, low, p - 1)
        quick_sort(arr, p + 1, high)

arr = [64, 25, 12, 22, 11]
print("Original array:", arr)

quick_sort(arr, 0, len(arr) - 1)
print("Sorted array:", arr)

OUTPUT:
Original array: [64, 25, 12, 22, 11]
Sorted array: [11, 12, 22, 25, 64]


H. Performance Comparison (Execution Time)

CODE:
import time
import random

def bubble_sort(arr):
    arr = arr[:]
    n = len(arr)
    for i in range(n - 1):
        for j in range(n - i - 1):
            if arr[j] > arr[j + 1]:
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
    return arr

def selection_sort(arr):
    arr = arr[:]
    n = len(arr)
    for i in range(n - 1):
        m = i
        for j in range(i + 1, n):
            if arr[j] < arr[m]:
                m = j
        arr[i], arr[m] = arr[m], arr[i]
    return arr

def insertion_sort(arr):
    arr = arr[:]
    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and arr[j] > key:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key
    return arr

def merge_sort(arr):
    if len(arr) <= 1:
        return arr[:]
    mid = len(arr) // 2
    left = merge_sort(arr[:mid])
    right = merge_sort(arr[mid:])
    result = []
    i = j = 0
    while i < len(left) and j < len(right):
        if left[i] <= right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1
    result.extend(left[i:])
    result.extend(right[j:])
    return result

def quick_sort(arr):
    arr = arr[:]
    def qs(low, high):
        if low < high:
            pivot = arr[high]
            i = low - 1
            for j in range(low, high):
                if arr[j] <= pivot:
                    i += 1
                    arr[i], arr[j] = arr[j], arr[i]
            arr[i + 1], arr[high] = arr[high], arr[i + 1]
            qs(low, i)
            qs(i + 2, high)
    qs(0, len(arr) - 1)
    return arr

data = [random.randint(1, 10000) for _ in range(2000)]

algorithms = [
    ("Bubble Sort", bubble_sort),
    ("Selection Sort", selection_sort),
    ("Insertion Sort", insertion_sort),
    ("Merge Sort", merge_sort),
    ("Quick Sort", quick_sort),
]

print("Sorting 2000 random elements:")
for name, func in algorithms:
    start = time.perf_counter()
    func(data)
    end = time.perf_counter()
    print(f"{name:<15}: {end - start:.4f} seconds")

OUTPUT:
Sorting 2000 random elements:
Bubble Sort    : 0.4692 seconds
Selection Sort : 0.1633 seconds
Insertion Sort : 0.2028 seconds
Merge Sort     : 0.0106 seconds
Quick Sort     : 0.0081 seconds



I. Performance Comparison Tables

Table 1: Execution time for sorting 2000 random elements

Sl. No.	Algorithm	Execution Time (seconds)
1	Bubble Sort	0.4692
2	Selection Sort	0.1633
3	Insertion Sort	0.2028
4	Merge Sort	0.0106
5	Quick Sort	0.0081

Table 2: Time and space complexity




Algorithm	Best Case	Average Case	Worst Case	Space Complexity
Bubble Sort	O(n²)	O(n²)	O(n²)	O(1)
Selection Sort	O(n²)	O(n²)	O(n²)	O(1)
Insertion Sort	O(n)	O(n²)	O(n²)	O(1)
Merge Sort	O(n log n)	O(n log n)	O(n log n)	O(n)
Quick Sort	O(n log n)	O(n log n)	O(n²)	O(log n) average

