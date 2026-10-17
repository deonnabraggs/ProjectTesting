import tkinter as tk
from tkinter import ttk, messagebox

class DeadlockSimulator:
    def __init__(self, root):
        self.root = root
        self.root.title("Interactive Deadlock Simulation System")
        self.root.geometry("1100x760")
        self.root.minsize(900, 650)

        self.n = 4
        self.m = 3
        self.resources = ["A", "B", "C"]
        self.processes = ["P0", "P1", "P2", "P3"]

        self.reset_data()
        self.build_ui()
        self.refresh()

    def reset_data(self):
        self.allocation = [
            [0, 1, 0],
            [2, 0, 0],
            [3, 0, 2],
            [2, 1, 1]
        ]
        self.maximum = [
            [7, 5, 3],
            [3, 2, 2],
            [9, 0, 2],
            [2, 2, 2]
        ]
        self.available = [3, 3, 2]
        self.request = [[0, 0, 0] for _ in range(self.n)]

    def build_ui(self):
        title = ttk.Label(
            self.root,
            text="INTERACTIVE DEADLOCK SIMULATION SYSTEM",
            font=("Arial", 17, "bold")
        )
        title.pack(pady=10)

        top = ttk.Frame(self.root)
        top.pack(fill="x", padx=10)

        self.status = ttk.Label(
            top, text="System Status: SAFE",
            font=("Arial", 12, "bold")
        )
        self.status.pack(side="left", padx=5)

        ttk.Button(
            top, text="Check Safe State",
            command=self.check_safe
        ).pack(side="left", padx=5)

        ttk.Button(
            top, text="Detect Deadlock",
            command=self.detect_deadlock
        ).pack(side="left", padx=5)

        ttk.Button(
            top, text="Reset",
            command=self.reset
        ).pack(side="left", padx=5)

        notebook = ttk.Notebook(self.root)
        notebook.pack(fill="both", expand=True, padx=10, pady=8)

        self.matrix_tab = ttk.Frame(notebook)
        self.request_tab = ttk.Frame(notebook)
        self.graph_tab = ttk.Frame(notebook)
        self.log_tab = ttk.Frame(notebook)

        notebook.add(self.matrix_tab, text="Resource Matrices")
        notebook.add(self.request_tab, text="Resource Requests")
        notebook.add(self.graph_tab, text="Resource Allocation Graph")
        notebook.add(self.log_tab, text="Event Log")

        self.build_matrices()
        self.build_requests()
        self.build_graph()
        self.build_log()

    def build_matrices(self):
        frame = ttk.Frame(self.matrix_tab)
        frame.pack(fill="both", expand=True, padx=10, pady=10)

        self.allocation_entries = []
        self.maximum_entries = []
        self.request_entries = []
        self.available_entries = []

        ttk.Label(
            frame, text="Allocation Matrix",
            font=("Arial", 11, "bold")
        ).grid(row=0, column=0, columnspan=4, pady=5)

        for j, resource in enumerate(self.resources):
            ttk.Label(frame, text=resource).grid(
                row=1, column=j + 1, padx=8
            )

        for i in range(self.n):
            ttk.Label(frame, text=self.processes[i]).grid(
                row=i + 2, column=0, padx=8
            )
            row = []
            for j in range(self.m):
                entry = ttk.Entry(frame, width=7)
                entry.grid(row=i + 2, column=j + 1, padx=4, pady=3)
                row.append(entry)
            self.allocation_entries.append(row)

        start = self.n + 3
        ttk.Label(
            frame, text="Maximum Matrix",
            font=("Arial", 11, "bold")
        ).grid(row=start, column=0, columnspan=4, pady=(15, 5))

        for j, resource in enumerate(self.resources):
            ttk.Label(frame, text=resource).grid(
                row=start + 1, column=j + 1, padx=8
            )

        for i in range(self.n):
            ttk.Label(frame, text=self.processes[i]).grid(
                row=start + i + 2, column=0, padx=8
            )
            row = []
            for j in range(self.m):
                entry = ttk.Entry(frame, width=7)
                entry.grid(
                    row=start + i + 2, column=j + 1, padx=4, pady=3
                )
                row.append(entry)
            self.maximum_entries.append(row)

        available_row = start + self.n + 3
        ttk.Label(
            frame, text="Available Resources:",
            font=("Arial", 10, "bold")
        ).grid(row=available_row, column=0, sticky="w", pady=10)

        for j in range(self.m):
            entry = ttk.Entry(frame, width=7)
            entry.grid(row=available_row, column=j + 1, padx=4)
            self.available_entries.append(entry)

        ttk.Button(
            frame, text="Update Matrices",
            command=self.update_matrices
        ).grid(
            row=available_row + 1, column=0,
            columnspan=4, pady=10
        )

        self.need_label = ttk.Label(
            frame, text="Need Matrix will appear here",
            font=("Arial", 10)
        )
        self.need_label.grid(
            row=available_row + 2, column=0,
            columnspan=4, sticky="w", pady=5
        )

    def build_requests(self):
        frame = ttk.Frame(self.request_tab)
        frame.pack(fill="both", expand=True, padx=15, pady=15)

        ttk.Label(
            frame, text="Request Resources",
            font=("Arial", 13, "bold")
        ).grid(row=0, column=0, columnspan=5, pady=10)

        ttk.Label(frame, text="Process:").grid(
            row=1, column=0, padx=5, pady=5
        )

        self.process_choice = ttk.Combobox(
            frame, values=self.processes,
            state="readonly", width=8
        )
        self.process_choice.current(0)
        self.process_choice.grid(row=1, column=1, padx=5)

        self.request_input = []
        for j, resource in enumerate(self.resources):
            ttk.Label(frame, text=resource).grid(
                row=2, column=j, padx=5, pady=5
            )
            entry = ttk.Entry(frame, width=8)
            entry.insert(0, "0")
            entry.grid(row=3, column=j, padx=5, pady=5)
            self.request_input.append(entry)

        ttk.Button(
            frame, text="Submit Request",
            command=self.submit_request
        ).grid(row=4, column=0, columnspan=3, pady=10)

        ttk.Label(
            frame,
            text="Enter the number of A, B and C resources requested."
        ).grid(row=5, column=0, columnspan=4, pady=5)

        ttk.Button(
            frame, text="Release Process Resources",
            command=self.recover
        ).grid(row=6, column=0, columnspan=3, pady=10)

        self.request_result = ttk.Label(
            frame, text="Request result will appear here",
            wraplength=600
        )
        self.request_result.grid(
            row=7, column=0, columnspan=4, pady=10
        )

    def build_graph(self):
        frame = ttk.Frame(self.graph_tab)
        frame.pack(fill="both", expand=True, padx=10, pady=10)

        ttk.Button(
            frame, text="Draw Resource Allocation Graph",
            command=self.draw_graph
        ).pack(pady=5)

        self.canvas = tk.Canvas(
            frame, width=850, height=440,
            bg="white", scrollregion=(0, 0, 850, 440)
        )
        self.canvas.pack(fill="both", expand=True)

    def build_log(self):
        frame = ttk.Frame(self.log_tab)
        frame.pack(fill="both", expand=True, padx=10, pady=10)

        self.log_box = tk.Text(
            frame, height=20, wrap="word",
            font=("Consolas", 10)
        )
        self.log_box.pack(fill="both", expand=True)

        ttk.Button(
            frame, text="Clear Log",
            command=lambda: self.log_box.delete("1.0", "end")
        ).pack(pady=5)

    def log(self, message):
        self.log_box.insert("end", message + "\n")
        self.log_box.see("end")

    def read_entry(self, entry):
        value = int(entry.get())
        if value < 0:
            raise ValueError("Values cannot be negative.")
        return value

    def update_matrices(self):
        try:
            allocation = [
                [self.read_entry(e) for e in row]
                for row in self.allocation_entries
            ]
            maximum = [
                [self.read_entry(e) for e in row]
                for row in self.maximum_entries
            ]
            available = [
                self.read_entry(e) for e in self.available_entries
            ]

            for i in range(self.n):
                for j in range(self.m):
                    if allocation[i][j] > maximum[i][j]:
                        raise ValueError(
                            f"Allocation cannot exceed Maximum "
                            f"for {self.processes[i]}, resource "
                            f"{self.resources[j]}."
                        )

            self.allocation = allocation
            self.maximum = maximum
            self.available = available

            self.refresh()
            self.log("Matrices updated successfully.")
            messagebox.showinfo("Success", "Matrices updated.")

        except ValueError as error:
            messagebox.showerror(
                "Invalid Input",
                "Enter valid non-negative integers.\n" + str(error)
            )

    def need_matrix(self):
        return [
            [
                self.maximum[i][j] - self.allocation[i][j]
                for j in range(self.m)
            ]
            for i in range(self.n)
        ]

    def safe_sequence(self):
        need = self.need_matrix()
        work = self.available[:]
        finished = [False] * self.n
        sequence = []

        while len(sequence) < self.n:
            found = False

            for i in range(self.n):
                if finished[i]:
                    continue

                if all(need[i][j] <= work[j] for j in range(self.m)):
                    for j in range(self.m):
                        work[j] += self.allocation[i][j]

                    finished[i] = True
                    sequence.append(self.processes[i])
                    found = True

            if not found:
                break

        return len(sequence) == self.n, sequence

    def check_safe(self):
        try:
            self.update_matrices_silent()
            safe, sequence = self.safe_sequence()

            if safe:
                self.status.config(text="System Status: SAFE")
                message = "System is SAFE.\nSafe sequence: " + " -> ".join(sequence)
                self.log(message)
                messagebox.showinfo("Safe State", message)
            else:
                self.status.config(text="System Status: UNSAFE")
                message = "System is UNSAFE. No complete safe sequence exists."
                self.log(message)
                messagebox.showwarning("Unsafe State", message)

        except ValueError as error:
            messagebox.showerror("Invalid Input", str(error))

    def update_matrices_silent(self):
        allocation = [
            [self.read_entry(e) for e in row]
            for row in self.allocation_entries
        ]
        maximum = [
            [self.read_entry(e) for e in row]
            for row in self.maximum_entries
        ]
        available = [
            self.read_entry(e) for e in self.available_entries
        ]

        for i in range(self.n):
            for j in range(self.m):
                if allocation[i][j] > maximum[i][j]:
                    raise ValueError(
                        f"Allocation cannot exceed Maximum for "
                        f"{self.processes[i]}, resource {self.resources[j]}."
                    )

        self.allocation = allocation
        self.maximum = maximum
        self.available = available
        self.refresh()

    def detect_deadlock(self):
        try:
            self.update_matrices_silent()
            work = self.available[:]
            finished = [
                all(value == 0 for value in self.allocation[i])
                for i in range(self.n)
            ]

            while True:
                found = False

                for i in range(self.n):
                    if finished[i]:
                        continue

                    if all(
                        self.request[i][j] <= work[j]
                        for j in range(self.m)
                    ):
                        for j in range(self.m):
                            work[j] += self.allocation[i][j]

                        finished[i] = True
                        found = True

                if not found:
                    break

            blocked = [
                self.processes[i]
                for i in range(self.n)
                if not finished[i]
            ]

            if blocked:
                self.status.config(text="System Status: DEADLOCK DETECTED")
                message = "Deadlock detected. Blocked processes: " + ", ".join(blocked)
                self.log(message)
                messagebox.showwarning("Deadlock", message)
            else:
                self.status.config(text="System Status: NO DEADLOCK")
                message = "No deadlock detected."
                self.log(message)
                messagebox.showinfo("Deadlock Detection", message)

        except ValueError as error:
            messagebox.showerror("Invalid Input", str(error))

    def submit_request(self):
        try:
            self.update_matrices_silent()
            i = self.processes.index(self.process_choice.get())
            request = [self.read_entry(e) for e in self.request_input]
            need = self.need_matrix()

            for j in range(self.m):
                if request[j] > need[i][j]:
                    raise ValueError(
                        f"Request exceeds the remaining Need of "
                        f"{self.processes[i]} for resource {self.resources[j]}."
                    )

                if request[j] > self.available[j]:
                    raise ValueError(
                        f"Not enough available resource {self.resources[j]}."
                    )

            old_allocation = [row[:] for row in self.allocation]
            old_available = self.available[:]

            for j in range(self.m):
                self.available[j] -= request[j]
                self.allocation[i][j] += request[j]

            safe, sequence = self.safe_sequence()

            if safe:
                self.request[i] = [0] * self.m
                self.refresh()
                self.draw_graph()
                message = (
                    f"Request granted to {self.processes[i]}.\n"
                    "System remains SAFE.\nSafe sequence: "
                    + " -> ".join(sequence)
                )
                self.request_result.config(text=message)
                self.log(message)
                messagebox.showinfo("Request Granted", message)
            else:
                self.allocation = old_allocation
                self.available = old_available
                message = (
                    "Request denied. Granting it would make the system unsafe."
                )
                self.request_result.config(text=message)
                self.log(message)
                messagebox.showwarning("Request Denied", message)

        except ValueError as error:
            messagebox.showerror("Invalid Request", str(error))

    def recover(self):
        i = self.processes.index(self.process_choice.get())

        for j in range(self.m):
            self.available[j] += self.allocation[i][j]
            self.allocation[i][j] = 0
            self.maximum[i][j] = 0
            self.request[i][j] = 0

        self.refresh()
        self.draw_graph()
        message = (
            f"Recovery: {self.processes[i]} terminated. "
            "Its allocated resources have been released."
        )
        self.request_result.config(text=message)
        self.log(message)
        messagebox.showinfo("Recovery", message)

    def refresh(self):
        for i in range(self.n):
            for j in range(self.m):
                self.allocation_entries[i][j].delete(0, "end")
                self.allocation_entries[i][j].insert(
                    0, str(self.allocation[i][j])
                )

                self.maximum_entries[i][j].delete(0, "end")
                self.maximum_entries[i][j].insert(
                    0, str(self.maximum[i][j])
                )

        for j in range(self.m):
            self.available_entries[j].delete(0, "end")
            self.available_entries[j].insert(
                0, str(self.available[j])
            )

        need = self.need_matrix()
        lines = ["Need Matrix (Maximum - Allocation):"]

        for i in range(self.n):
            lines.append(f"{self.processes[i]}: {need[i]}")

        lines.append(f"Available: {self.available}")
        self.need_label.config(text="\n".join(lines))
        self.draw_graph()

    def draw_graph(self):
        self.canvas.delete("all")

        process_positions = [
            (140, 90), (140, 190), (140, 290), (140, 390)
        ]
        resource_positions = [
            (620, 130), (620, 240), (620, 350)
        ]

        # Resource allocation edges: Resource -> Process
        for i in range(self.n):
            for j in range(self.m):
                if self.allocation[i][j] > 0:
                    x1, y1 = resource_positions[j]
                    x2, y2 = process_positions[i]
                    self.canvas.create_line(
                        x1 - 35, y1, x2 + 35, y2,
                        arrow=tk.LAST, fill="blue", width=2
                    )
                    self.canvas.create_text(
                        (x1 + x2) // 2, (y1 + y2) // 2 - 8,
                        text=str(self.allocation[i][j]),
                        fill="blue"
                    )

        # Request edges: Process -> Resource
        for i in range(self.n):
            for j in range(self.m):
                if self.request[i][j] > 0:
                    x1, y1 = process_positions[i]
                    x2, y2 = resource_positions[j]
                    self.canvas.create_line(
                        x1 + 35, y1, x2 - 35, y2,
                        arrow=tk.LAST, fill="red", width=2,
                        dash=(5, 3)
                    )
                    self.canvas.create_text(
                        (x1 + x2) // 2, (y1 + y2) // 2 + 8,
                        text=str(self.request[i][j]), fill="red"
                    )

        for i, (x, y) in enumerate(process_positions):
            self.canvas.create_oval(
                x - 35, y - 25, x + 35, y + 25,
                fill="lightblue", outline="black"
            )
            self.canvas.create_text(
                x, y, text=self.processes[i], font=("Arial", 10, "bold")
            )

        for j, (x, y) in enumerate(resource_positions):
            self.canvas.create_rectangle(
                x - 35, y - 25, x + 35, y + 25,
                fill="lightyellow", outline="black"
            )
            self.canvas.create_text(
                x, y, text=f"{self.resources[j]} ({self.available[j]} free)",
                font=("Arial", 10, "bold")
            )

        self.canvas.create_text(
            400, 20,
            text="Blue: allocated resources    Red dashed: pending requests",
            font=("Arial", 10)
        )

    def reset(self):
        if messagebox.askyesno(
            "Reset", "Restore the original sample data?"
        ):
            self.reset_data()
            self.status.config(text="System Status: SAFE")
            self.request_result.config(
                text="Request result will appear here"
            )
            self.refresh()
            self.log("Sample data restored.")

if __name__ == "__main__":
    root = tk.Tk()
    app = DeadlockSimulator(root)
    root.mainloop()
