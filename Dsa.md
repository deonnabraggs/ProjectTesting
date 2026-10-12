QUESTION: Implement operations on Lists, Tuples, Sets, and Dictionaries, including insertion, deletion, slicing, and comprehensions.

A. List Operations

CODE:
a = [10, 20, 30, 40]
print("List:", a)

a.append(50)
print("After insertion:", a)

a.remove(20)
print("After deletion:", a)

print("Slicing:", a[1:3])

squares = [x*x for x in a]
print("Comprehension:", squares)

OUTPUT:
List: [10, 20, 30, 40]
After insertion: [10, 20, 30, 40, 50]
After deletion: [10, 30, 40, 50]
Slicing: [30, 40]
Comprehension: [100, 900, 1600, 2500]


B. Tuple Operations

CODE:
a = (10, 20, 30, 40)
print("Tuple:", a)
print("Slicing:", a[1:3])
print("Length:", len(a))
print("Element:", a[2])

OUTPUT:
Tuple: (10, 20, 30, 40)
Slicing: (20, 30)
Length: 4
Element: 30


C. Set Operations

CODE:
a = {10, 20, 30}
b = {30, 40, 50}

a.add(40)
print("After insertion:", sorted(a))

a.discard(20)
print("After deletion:", sorted(a))

print("Union:", sorted(a | b))
print("Intersection:", sorted(a & b))

OUTPUT:
After insertion: [10, 20, 30, 40]
After deletion: [10, 30, 40]
Union: [10, 30, 40, 50]
Intersection: [30, 40]


D. Dictionary Operations

CODE:
a = {"A": 10, "B": 20}
print("Dictionary:", a)

a["C"] = 30
print("After insertion:", a)

del a["B"]
print("After deletion:", a)

squares = {x: x*x for x in range(1, 4)}
print("Comprehension:", squares)

OUTPUT:
Dictionary: {'A': 10, 'B': 20}
After insertion: {'A': 10, 'B': 20, 'C': 30}
After deletion: {'A': 10, 'C': 30}
Comprehension: {1: 1, 2: 4, 3: 9}


==================================================

PROGRAM 2

QUESTION: Perform string manipulation using slicing and comprehensions.

A. String Slicing

CODE:
s = "Python Programming"

print("Original string:", s)
print("First six characters:", s[:6])
print("Last eleven characters:", s[7:])
print("Reverse string:", s[::-1])

OUTPUT:
Original string: Python Programming
First six characters: Python
Last eleven characters: Programming
Reverse string: gnimmargorP nohtyP


B. String Comprehension

CODE:
s = "Python Programming"

vowels = [ch for ch in s if ch.lower() in "aeiou"]
uppercase = [ch for ch in s if ch.isupper()]
letters = [ch for ch in s if ch.isalpha()]

print("Vowels:", vowels)
print("Uppercase letters:", uppercase)
print("Alphabet count:", len(letters))

OUTPUT:
Vowels: ['o', 'o', 'a', 'i']
Uppercase letters: ['P']
Alphabet count: 17


==================================================

PROGRAM 3

QUESTION: Implement one-dimensional and two-dimensional arrays with traversal, insertion, and deletion.

A. One-Dimensional Array

CODE:
import array as arr

a = arr.array('i', [10, 20, 30, 40])
print("Original array:", a.tolist())

a.insert(2, 25)
print("After insertion:", a.tolist())

a.remove(20)
print("After deletion:", a.tolist())

print("Array traversal:")
for x in a:
    print(x, end=" ")

OUTPUT:
Original array: [10, 20, 30, 40]
After insertion: [10, 20, 25, 30, 40]
After deletion: [10, 25, 30, 40]
Array traversal:
10 25 30 40


B. Two-Dimensional Array

CODE:
a = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
]

print("Original array:")
for row in a:
    print(row)

a[1].insert(1, 50)
print("After insertion:")
for row in a:
    print(row)

del a[1][1]
print("After deletion:")
for row in a:
    print(row)

OUTPUT:
Original array:
[1, 2, 3]
[4, 5, 6]
[7, 8, 9]
After insertion:
[1, 2, 3]
[4, 50, 5, 6]
[7, 8, 9]
After deletion:
[1, 2, 3]
[4, 5, 6]
[7, 8, 9]


==================================================

PROGRAM 4

QUESTION: Implement searching and sorting algorithms and compare their time complexities.

A. Linear Search

CODE:
import array as arr

a = arr.array('i', [10, 20, 30, 40, 50])
key = int(input("Enter element to search: "))
found = False

for i in range(len(a)):
    if a[i] == key:
        print("Element found at index:", i)
        found = True
        break

if not found:
    print("Element not found")

OUTPUT:
Enter element to search: 30
Element found at index: 2


B. Binary Search

CODE:
a = [10, 20, 30, 40, 50]
key = int(input("Enter element to search: "))

low = 0
high = len(a) - 1
found = False

while low <= high:
    mid = (low + high) // 2

    if a[mid] == key:
        print("Element found at index:", mid)
        found = True
        break
    elif a[mid] < key:
        low = mid + 1
    else:
        high = mid - 1

if not found:
    print("Element not found")

OUTPUT:
Enter element to search: 40
Element found at index: 3


C. Bubble Sort

CODE:
a = [50, 20, 40, 10, 30]

for i in range(len(a)):
    for j in range(len(a) - i - 1):
        if a[j] > a[j + 1]:
            a[j], a[j + 1] = a[j + 1], a[j]

print("Sorted array:", a)

OUTPUT:
Sorted array: [10, 20, 30, 40, 50]


D. Selection Sort

CODE:
a = [50, 20, 40, 10, 30]

for i in range(len(a)):
    min_index = i

    for j in range(i + 1, len(a)):
        if a[j] < a[min_index]:
            min_index = j

    a[i], a[min_index] = a[min_index], a[i]

print("Sorted array:", a)

OUTPUT:
Sorted array: [10, 20, 30, 40, 50]


E. Insertion Sort

CODE:
a = [50, 20, 40, 10, 30]

for i in range(1, len(a)):
    key = a[i]
    j = i - 1

    while j >= 0 and a[j] > key:
        a[j + 1] = a[j]
        j -= 1

    a[j + 1] = key

print("Sorted array:", a)

OUTPUT:
Sorted array: [10, 20, 30, 40, 50]


F. Merge Sort

CODE:
def merge_sort(a):
    if len(a) > 1:
        mid = len(a) // 2
        left = a[:mid]
        right = a[mid:]

        merge_sort(left)
        merge_sort(right)

        i = j = k = 0

        while i < len(left) and j < len(right):
            if left[i] < right[j]:
                a[k] = left[i]
                i += 1
            else:
                a[k] = right[j]
                j += 1
            k += 1

        while i < len(left):
            a[k] = left[i]
            i += 1
            k += 1

        while j < len(right):
            a[k] = right[j]
            j += 1
            k += 1

a = [50, 20, 40, 10, 30]
merge_sort(a)
print("Sorted array:", a)

OUTPUT:
Sorted array: [10, 20, 30, 40, 50]


G. Quick Sort

CODE:
def quick_sort(a):
    if len(a) <= 1:
        return a

    pivot = a[len(a) // 2]
    left = [x for x in a if x < pivot]
    middle = [x for x in a if x == pivot]
    right = [x for x in a if x > pivot]

    return quick_sort(left) + middle + quick_sort(right)

a = [50, 20, 40, 10, 30]
print("Sorted array:", quick_sort(a))

OUTPUT:
Sorted array: [10, 20, 30, 40, 50]


H. Performance Comparison

| Algorithm | Best Case | Average Case | Worst Case |
|---|---|---|---|
| Linear Search | O(1) | O(n) | O(n) |
| Binary Search | O(1) | O(log n) | O(log n) |
| Bubble Sort | O(n²) | O(n²) | O(n²) |
| Selection Sort | O(n²) | O(n²) | O(n²) |
| Insertion Sort | O(n) | O(n²) | O(n²) |
| Merge Sort | O(n log n) | O(n log n) | O(n log n) |
| Quick Sort | O(n log n) | O(n log n) | O(n²) |


==================================================
