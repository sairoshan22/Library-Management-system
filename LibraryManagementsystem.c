#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_STR 100
#define FINE_RATE_PER_DAY 5.0f

// Structures as specified in Sections 7 & 8
typedef struct {
    int book_id;
    char title[MAX_STR];
    char author[MAX_STR];
    int quantity;
} Book;

typedef struct {
    int issue_id;
    int book_id;
    int user_id;
    char user_name[MAX_STR];
    char issue_date[11]; // YYYY-MM-DD
    char due_date[11];   // YYYY-MM-DD
    char return_date[11];// YYYY-MM-DD or "N/A"
    float fine_amount;
    int is_returned;    // 0 = Issued, 1 = Returned
} IssueRecord;

// Global Data Arrays & Counters
Book *books = NULL;
int book_count = 0;

IssueRecord *issues = NULL;
int issue_count = 0;

// Helper Functions
void str_to_lowercase(char *str) {
    for (; *str; ++str) *str = (char)tolower((unsigned char)*str);
}

int case_insensitive_match(const char *s1, const char *s2) {
    char t1[MAX_STR], t2[MAX_STR];
    strncpy(t1, s1, MAX_STR - 1); t1[MAX_STR - 1] = '\0';
    strncpy(t2, s2, MAX_STR - 1); t2[MAX_STR - 1] = '\0';
    str_to_lowercase(t1);
    str_to_lowercase(t2);
    return strcmp(t1, t2) == 0;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_current_date(char *buffer) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buffer, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

void calculate_due_date(const char *issue_date_str, char *due_date_str, int days_to_add) {
    struct tm tm_date = {0};
    sscanf(issue_date_str, "%d-%d-%d", &tm_date.tm_year, &tm_date.tm_mon, &tm_date.tm_mday);
    tm_date.tm_year -= 1900;
    tm_date.tm_mon -= 1;
    
    time_t t = mktime(&tm_date);
    t += (days_to_add * 24 * 60 * 60);
    
    struct tm *tm_due = localtime(&t);
    sprintf(due_date_str, "%04d-%02d-%02d", tm_due->tm_year + 1900, tm_due->tm_mon + 1, tm_due->tm_mday);
}

int calculate_days_difference(const char *date1_str, const char *date2_str) {
    struct tm tm1 = {0}, tm2 = {0};
    sscanf(date1_str, "%d-%d-%d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday);
    sscanf(date2_str, "%d-%d-%d", &tm2.tm_year, &tm2.tm_mon, &tm2.tm_mday);
    
    tm1.tm_year -= 1900; tm1.tm_mon -= 1;
    tm2.tm_year -= 1900; tm2.tm_mon -= 1;

    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);

    double diff_seconds = difftime(t2, t1);
    return (int)(diff_seconds / (60 * 60 * 24));
}

// File I/O
void load_data() {
    FILE *fp_books = fopen("books.dat", "rb");
    if (fp_books) {
        fread(&book_count, sizeof(int), 1, fp_books);
        if (book_count > 0) {
            books = (Book *)malloc(sizeof(Book) * book_count);
            fread(books, sizeof(Book), book_count, fp_books);
        }
        fclose(fp_books);
    }

    FILE *fp_issues = fopen("issued_books.dat", "rb");
    if (fp_issues) {
        fread(&issue_count, sizeof(int), 1, fp_issues);
        if (issue_count > 0) {
            issues = (IssueRecord *)malloc(sizeof(IssueRecord) * issue_count);
            fread(issues, sizeof(IssueRecord), issue_count, fp_issues);
        }
        fclose(fp_issues);
    }
}

void save_data() {
    FILE *fp_books = fopen("books.dat", "wb");
    if (fp_books) {
        fwrite(&book_count, sizeof(int), 1, fp_books);
        if (book_count > 0) {
            fwrite(books, sizeof(Book), book_count, fp_books);
        }
        fclose(fp_books);
    }

    FILE *fp_issues = fopen("issued_books.dat", "wb");
    if (fp_issues) {
        fwrite(&issue_count, sizeof(int), 1, fp_issues);
        if (issue_count > 0) {
            fwrite(issues, sizeof(IssueRecord), issue_count, fp_issues);
        }
        fclose(fp_issues);
    }
    printf("\n[SUCCESS] All data saved successfully!\n");
}

void free_memory() {
    if (books) free(books);
    if (issues) free(issues);
}

// Menu Functions
void add_book() {
    Book new_book;
    printf("\n--- Add New Book ---\n");
    printf("Enter Book ID: ");
    if (scanf("%d", &new_book.book_id) != 1) {
        printf("[ERROR] Invalid Book ID.\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    for (int i = 0; i < book_count; i++) {
        if (books[i].book_id == new_book.book_id) {
            printf("[ERROR] Book ID %d already exists!\n", new_book.book_id);
            return;
        }
    }

    printf("Enter Book Title: ");
    fgets(new_book.title, MAX_STR, stdin);
    new_book.title[strcspn(new_book.title, "\n")] = 0;

    printf("Enter Author Name: ");
    fgets(new_book.author, MAX_STR, stdin);
    new_book.author[strcspn(new_book.author, "\n")] = 0;

    printf("Enter Quantity: ");
    if (scanf("%d", &new_book.quantity) != 1 || new_book.quantity < 0) {
        printf("[ERROR] Invalid Quantity!\n");
        clear_input_buffer();
        return;
    }

    books = (Book *)realloc(books, sizeof(Book) * (book_count + 1));
    books[book_count++] = new_book;

    printf("[SUCCESS] Book added successfully!\n");
}

void update_book_menu() {
    if (book_count == 0) {
        printf("\n[INFO] No books available.\n");
        return;
    }

    char choice;
    printf("\n--- Update Book Details ---\n");
    printf("A. By Book ID\n");
    printf("B. By Book Name\n");
    printf("C. Back to Main Menu\n");
    printf("Select an Option: ");
    scanf(" %c", &choice);

    int target_index = -1;

    if (choice == 'A' || choice == 'a') {
        int id;
        printf("Enter Book ID: ");
        scanf("%d", &id);
        for (int i = 0; i < book_count; i++) {
            if (books[i].book_id == id) {
                target_index = i;
                break;
            }
        }
    } else if (choice == 'B' || choice == 'b') {
        char name[MAX_STR];
        printf("Enter Book Name: ");
        clear_input_buffer();
        fgets(name, MAX_STR, stdin);
        name[strcspn(name, "\n")] = 0;

        for (int i = 0; i < book_count; i++) {
            if (case_insensitive_match(books[i].title, name)) {
                target_index = i;
                break;
            }
        }
    } else if (choice == 'C' || choice == 'c') {
        return;
    } else {
        printf("[ERROR] Invalid choice.\n");
        return;
    }

    if (target_index == -1) {
        printf("[ERROR] Book not found.\n");
        return;
    }

    clear_input_buffer();
    printf("\nUpdating Book ID %d\n", books[target_index].book_id);
    printf("Enter New Title: ");
    fgets(books[target_index].title, MAX_STR, stdin);
    books[target_index].title[strcspn(books[target_index].title, "\n")] = 0;

    printf("Enter New Author: ");
    fgets(books[target_index].author, MAX_STR, stdin);
    books[target_index].author[strcspn(books[target_index].author, "\n")] = 0;

    printf("Enter New Quantity: ");
    scanf("%d", &books[target_index].quantity);

    printf("[SUCCESS] Book updated successfully.\n");
}

void remove_book_menu() {
    if (book_count == 0) {
        printf("\n[INFO] No books to remove.\n");
        return;
    }

    char choice;
    printf("\n--- Remove Book Submenu ---\n");
    printf("A. By Book ID\n");
    printf("B. By Book Name\n");
    printf("C. Back to Main Menu\n");
    printf("Select an Option: ");
    scanf(" %c", &choice);

    int target_index = -1;

    if (choice == 'A' || choice == 'a') {
        int id;
        printf("Enter Book ID: ");
        scanf("%d", &id);
        for (int i = 0; i < book_count; i++) {
            if (books[i].book_id == id) {
                target_index = i;
                break;
            }
        }
    } else if (choice == 'B' || choice == 'b') {
        char name[MAX_STR];
        printf("Enter Book Name: ");
        clear_input_buffer();
        fgets(name, MAX_STR, stdin);
        name[strcspn(name, "\n")] = 0;

        for (int i = 0; i < book_count; i++) {
            if (case_insensitive_match(books[i].title, name)) {
                target_index = i;
                break;
            }
        }
    } else if (choice == 'C' || choice == 'c') {
        return;
    } else {
        printf("[ERROR] Invalid option.\n");
        return;
    }

    if (target_index == -1) {
        printf("[ERROR] Book not found.\n");
        return;
    }

    for (int i = target_index; i < book_count - 1; i++) {
        books[i] = books[i + 1];
    }
    book_count--;
    books = (Book *)realloc(books, sizeof(Book) * book_count);

    printf("[SUCCESS] Book removed.\n");
}

void search_book_menu() {
    if (book_count == 0) {
        printf("\n[INFO] No books in records.\n");
        return;
    }

    char choice;
    printf("\n--- Search Book Submenu ---\n");
    printf("A. By Book ID\n");
    printf("B. By Book Name\n");
    printf("C. By Author Name\n");
    printf("D. Back to Main Menu\n");
    printf("Select an Option: ");
    scanf(" %c", &choice);

    int found = 0;
    if (choice == 'A' || choice == 'a') {
        int id;
        printf("Enter Book ID: ");
        scanf("%d", &id);
        printf("\n%-10s %-30s %-25s %-10s\n", "ID", "Title", "Author", "Quantity");
        printf("-----------------------------------------------------------------------\n");
        for (int i = 0; i < book_count; i++) {
            if (books[i].book_id == id) {
                printf("%-10d %-30s %-25s %-10d\n", books[i].book_id, books[i].title, books[i].author, books[i].quantity);
                found = 1;
                break;
            }
        }
    } else if (choice == 'B' || choice == 'b') {
        char query[MAX_STR], temp_title[MAX_STR], temp_query[MAX_STR];
        printf("Enter Book Name (or Partial): ");
        clear_input_buffer();
        fgets(query, MAX_STR, stdin);
        query[strcspn(query, "\n")] = 0;

        strcpy(temp_query, query);
        str_to_lowercase(temp_query);

        printf("\n%-10s %-30s %-25s %-10s\n", "ID", "Title", "Author", "Quantity");
        printf("-----------------------------------------------------------------------\n");
        for (int i = 0; i < book_count; i++) {
            strcpy(temp_title, books[i].title);
            str_to_lowercase(temp_title);
            if (strstr(temp_title, temp_query) != NULL) {
                printf("%-10d %-30s %-25s %-10d\n", books[i].book_id, books[i].title, books[i].author, books[i].quantity);
                found = 1;
            }
        }
    } else if (choice == 'C' || choice == 'c') {
        char query[MAX_STR], temp_author[MAX_STR], temp_query[MAX_STR];
        printf("Enter Author Name (or Partial): ");
        clear_input_buffer();
        fgets(query, MAX_STR, stdin);
        query[strcspn(query, "\n")] = 0;

        strcpy(temp_query, query);
        str_to_lowercase(temp_query);

        printf("\n%-10s %-30s %-25s %-10s\n", "ID", "Title", "Author", "Quantity");
        printf("-----------------------------------------------------------------------\n");
        for (int i = 0; i < book_count; i++) {
            strcpy(temp_author, books[i].author);
            str_to_lowercase(temp_author);
            if (strstr(temp_author, temp_query) != NULL) {
                printf("%-10d %-30s %-25s %-10d\n", books[i].book_id, books[i].title, books[i].author, books[i].quantity);
                found = 1;
            }
        }
    } else if (choice == 'D' || choice == 'd') {
        return;
    } else {
        printf("[ERROR] Invalid choice.\n");
        return;
    }

    if (!found) {
        printf("[INFO] No matching records found.\n");
    }
}

void view_all_books() {
    if (book_count == 0) {
        printf("\n[INFO] No books available.\n");
        return;
    }

    printf("\n--- All Book Records ---\n");
    printf("%-10s %-30s %-25s %-10s\n", "Book ID", "Title", "Author", "Quantity");
    printf("-----------------------------------------------------------------------\n");
    for (int i = 0; i < book_count; i++) {
        printf("%-10d %-30s %-25s %-10d\n", books[i].book_id, books[i].title, books[i].author, books[i].quantity);
    }
}

void issue_book() {
    int b_id, u_id;
    printf("\n--- Issue Book Process ---\n");
    printf("Enter Book ID to Issue: ");
    if (scanf("%d", &b_id) != 1) {
        printf("[ERROR] Invalid Book ID.\n");
        clear_input_buffer();
        return;
    }

    int book_index = -1;
    for (int i = 0; i < book_count; i++) {
        if (books[i].book_id == b_id) {
            book_index = i;
            break;
        }
    }

    if (book_index == -1) {
        printf("[ERROR] Book ID %d not found.\n", b_id);
        return;
    }

    // FIXED: Corrected books[i].quantity to books[book_index].quantity
    if (books[book_index].quantity <= 0) {
        printf("[ERROR] Book '%s' is out of stock.\n", books[book_index].title);
        return;
    }

    IssueRecord new_issue;
    new_issue.issue_id = issue_count + 1001;
    new_issue.book_id = b_id;

    printf("Enter Borrower User ID: ");
    if (scanf("%d", &u_id) != 1) {
        printf("[ERROR] Invalid User ID.\n");
        clear_input_buffer();
        return;
    }
    new_issue.user_id = u_id;

    clear_input_buffer();
    printf("Enter Borrower Name: ");
    fgets(new_issue.user_name, MAX_STR, stdin);
    new_issue.user_name[strcspn(new_issue.user_name, "\n")] = 0;

    get_current_date(new_issue.issue_date);
    calculate_due_date(new_issue.issue_date, new_issue.due_date, 7);
    strcpy(new_issue.return_date, "N/A");
    new_issue.fine_amount = 0.0f;
    new_issue.is_returned = 0;

    books[book_index].quantity--;

    issues = (IssueRecord *)realloc(issues, sizeof(IssueRecord) * (issue_count + 1));
    issues[issue_count++] = new_issue;

    printf("[SUCCESS] Book Issued Successfully!\n");
    printf("Issue ID: %d | Due Date: %s\n", new_issue.issue_id, new_issue.due_date);
}

void return_book() {
    int b_id, u_id;
    printf("\n--- Return Book Process ---\n");
    printf("Enter Book ID: ");
    scanf("%d", &b_id);
    printf("Enter User ID: ");
    scanf("%d", &u_id);

    int issue_index = -1;
    for (int i = 0; i < issue_count; i++) {
        if (issues[i].book_id == b_id && issues[i].user_id == u_id && issues[i].is_returned == 0) {
            issue_index = i;
            break;
        }
    }

    if (issue_index == -1) {
        printf("[ERROR] Active issue record not found.\n");
        return;
    }

    get_current_date(issues[issue_index].return_date);

    int late_days = calculate_days_difference(issues[issue_index].due_date, issues[issue_index].return_date);
    if (late_days > 0) {
        issues[issue_index].fine_amount = late_days * FINE_RATE_PER_DAY;
        printf("[ALERT] Returned %d days late. Fine: ₹%.2f\n", late_days, issues[issue_index].fine_amount);
    } else {
        issues[issue_index].fine_amount = 0.0f;
        printf("[INFO] Returned on time. No fine.\n");
    }

    issues[issue_index].is_returned = 1;

    for (int i = 0; i < book_count; i++) {
        if (books[i].book_id == b_id) {
            books[i].quantity++;
            break;
        }
    }

    printf("[SUCCESS] Book returned successfully.\n");
}

void list_issued_books() {
    if (issue_count == 0) {
        printf("\n[INFO] No issued books.\n");
        return;
    }

    printf("\n--- Issued Books Log ---\n");
    printf("%-8s %-8s %-8s %-18s %-12s %-12s %-12s %-8s\n", 
           "Iss_ID", "B_ID", "U_ID", "User Name", "Issue Date", "Due Date", "Return Date", "Fine (₹)");
    printf("--------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < issue_count; i++) {
        printf("%-8d %-8d %-8d %-18s %-12s %-12s %-12s ₹%-7.2f\n", 
               issues[i].issue_id, 
               issues[i].book_id, 
               issues[i].user_id, 
               issues[i].user_name, 
               issues[i].issue_date, 
               issues[i].due_date, 
               issues[i].return_date, 
               issues[i].fine_amount);
    }
}

int main() {
    load_data();
    int choice;
    while (1) {
        printf("\n+----------------------------------------+\n");
        printf("|         Book Management Menu           |\n");
        printf("|----------------------------------------|\n");
        printf("| 1. Add New Book                        |\n");
        printf("| 2. Update Book Details                 |\n");
        printf("| 3. Remove Book                         |\n");
        printf("| 4. Search Book                         |\n");
        printf("| 5. View All Books                      |\n");
        printf("| 6. Issue Book                          |\n");
        printf("| 7. Return Book                         |\n");
        printf("| 8. List Issued Books                   |\n");
        printf("| 9. Save                                |\n");
        printf("| 10. Exit                               |\n");
        printf("+----------------------------------------+\n");
        printf("Select an Option (1-10): ");
        
        if (scanf("%d", &choice) != 1) {
            printf("[ERROR] Invalid option.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice) {
            case 1: add_book(); break;
            case 2: update_book_menu(); break;
            case 3: remove_book_menu(); break;
            case 4: search_book_menu(); break;
            case 5: view_all_books(); break;
            case 6: issue_book(); break;
            case 7: return_book(); break;
            case 8: list_issued_books(); break;
            case 9: save_data(); break;
            case 10:
                save_data();
                free_memory();
                printf("\nExiting System. Goodbye!\n");
                return 0;
            default:
                printf("[ERROR] Choice out of range (1-10).\n");
        }
    }
    return 0;
}
