Library Management System in C

A simple C project for managing library books, issuing/returning records, calculating fines, and storing data in files.

What It Does

* Book Management: Add, update, delete, search, and view book details.
* Issue & Return: Tracks who took which book, automatically creates issue and due dates (7-day period).
* Late Fine Calculation: Calculates ₹5/day fine if returned past the due date.
* Data Saving: Saves book catalog and borrow records to binary files (`books.dat` and `issued_books.dat`).

System Options:

1. Add New Book
2. Update Book Details (by ID / Name)
3. Remove Book (by ID / Name)
4. Search Book (by ID / Name / Author)
5. View All Books
6. Issue Book
7. Return Book
8. List Issued Books
9. Save
10. Exit

Rules Built In

*Due Date**: Fixed at 7 days from the date of issue.

*Fine**: ₹5 for each day overdue.

*Stock Count**: Decreases by 1 when issued, increases by 1 when returned.


How to Run

1. Open terminal in the project folder.

2. Compile the program:
bash
gcc main.c -o lms

3. Run the application:
bash
./lms
