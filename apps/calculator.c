#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define MAX_EXPRESSION 256
#define MAX_HISTORY 10

typedef struct {
    double result;
    char expression[MAX_EXPRESSION];
    time_t timestamp;
} calculation_t;

typedef struct {
    calculation_t history[MAX_HISTORY];
    int history_count;
    int current_index;
} calculator_t;

// Initialize calculator
void calculator_init(calculator_t* calc) {
    memset(calc, 0, sizeof(calculator_t));
    calc->current_index = -1;
}

// Add calculation to history
void calculator_add_to_history(calculator_t* calc, const char* expression, double result) {
    // Shift history if full
    if (calc->history_count >= MAX_HISTORY) {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            calc->history[i] = calc->history[i + 1];
        }
        calc->history_count = MAX_HISTORY - 1;
    }
    
    // Add new calculation
    calculation_t* calc_entry = &calc->history[calc->history_count];
    strncpy(calc_entry->expression, expression, MAX_EXPRESSION - 1);
    calc_entry->expression[MAX_EXPRESSION - 1] = '\0';
    calc_entry->result = result;
    calc_entry->timestamp = time(NULL);
    
    calc->history_count++;
    calc->current_index = calc->history_count - 1;
}

// Display calculator interface
void calculator_display(calculator_t* calc) {
    printf("\033[2J\033[H"); // Clear screen
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    RaeenOS Calculator v1.0                   ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  Operations: +, -, *, /, ^, sqrt(), sin(), cos(), tan()      ║\n");
    printf("║  Constants: pi, e                                            ║\n");
    printf("║  Commands: 'history', 'clear', 'quit'                        ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

// Parse and evaluate mathematical expression
double calculator_evaluate(const char* expression) {
    if (!expression || strlen(expression) == 0) {
        return 0.0;
    }
    
    // Simple expression parser (basic implementation)
    // In a real calculator, you'd use a proper parser
    
    char* expr_copy = strdup(expression);
    if (!expr_copy) {
        return 0.0;
    }
    
    // Remove spaces
    char* p = expr_copy;
    char* q = expr_copy;
    while (*p) {
        if (*p != ' ') {
            *q++ = *p;
        }
        p++;
    }
    *q = '\0';
    
    // Handle special functions and constants
    if (strcmp(expr_copy, "pi") == 0) {
        free(expr_copy);
        return M_PI;
    }
    
    if (strcmp(expr_copy, "e") == 0) {
        free(expr_copy);
        return M_E;
    }
    
    // Handle sqrt function
    if (strncmp(expr_copy, "sqrt(", 5) == 0) {
        int len = strlen(expr_copy);
        if (expr_copy[len - 1] == ')') {
            expr_copy[len - 1] = '\0';
            double arg = calculator_evaluate(expr_copy + 5);
            free(expr_copy);
            return sqrt(arg);
        }
    }
    
    // Handle trigonometric functions
    if (strncmp(expr_copy, "sin(", 4) == 0) {
        int len = strlen(expr_copy);
        if (expr_copy[len - 1] == ')') {
            expr_copy[len - 1] = '\0';
            double arg = calculator_evaluate(expr_copy + 4);
            free(expr_copy);
            return sin(arg);
        }
    }
    
    if (strncmp(expr_copy, "cos(", 4) == 0) {
        int len = strlen(expr_copy);
        if (expr_copy[len - 1] == ')') {
            expr_copy[len - 1] = '\0';
            double arg = calculator_evaluate(expr_copy + 4);
            free(expr_copy);
            return cos(arg);
        }
    }
    
    if (strncmp(expr_copy, "tan(", 4) == 0) {
        int len = strlen(expr_copy);
        if (expr_copy[len - 1] == ')') {
            expr_copy[len - 1] = '\0';
            double arg = calculator_evaluate(expr_copy + 4);
            free(expr_copy);
            return tan(arg);
        }
    }
    
    // Simple arithmetic evaluation
    // This is a very basic implementation
    // A real calculator would use a proper expression parser
    
    double result = 0.0;
    char* token = strtok(expr_copy, "+-*/^");
    if (token) {
        result = atof(token);
        
        char* op = strpbrk(expr_copy, "+-*/^");
        while (op) {
            token = strtok(NULL, "+-*/^");
            if (!token) break;
            
            double operand = atof(token);
            
            switch (*op) {
                case '+':
                    result += operand;
                    break;
                case '-':
                    result -= operand;
                    break;
                case '*':
                    result *= operand;
                    break;
                case '/':
                    if (operand != 0.0) {
                        result /= operand;
                    } else {
                        printf("Error: Division by zero\n");
                        free(expr_copy);
                        return 0.0;
                    }
                    break;
                case '^':
                    result = pow(result, operand);
                    break;
            }
            
            op = strpbrk(op + 1, "+-*/^");
        }
    }
    
    free(expr_copy);
    return result;
}

// Display calculation history
void calculator_show_history(calculator_t* calc) {
    printf("\nCalculation History:\n");
    printf("══════════════════════════════════════════════════════════════\n");
    
    if (calc->history_count == 0) {
        printf("No calculations in history.\n");
        return;
    }
    
    for (int i = 0; i < calc->history_count; i++) {
        calculation_t* entry = &calc->history[i];
        printf("%2d. %s = %.6g\n", i + 1, entry->expression, entry->result);
    }
    
    printf("══════════════════════════════════════════════════════════════\n");
}

// Clear history
void calculator_clear_history(calculator_t* calc) {
    calc->history_count = 0;
    calc->current_index = -1;
    printf("History cleared.\n");
}

// Main calculator loop
void calculator_run(calculator_t* calc) {
    char input[MAX_EXPRESSION];
    
    while (1) {
        printf("\nEnter expression (or command): ");
        
        if (!fgets(input, MAX_EXPRESSION, stdin)) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = '\0';
        
        // Handle commands
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        if (strcmp(input, "history") == 0) {
            calculator_show_history(calc);
            continue;
        }
        
        if (strcmp(input, "clear") == 0) {
            calculator_clear_history(calc);
            continue;
        }
        
        if (strcmp(input, "help") == 0) {
            calculator_display(calc);
            continue;
        }
        
        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }
        
        // Evaluate expression
        double result = calculator_evaluate(input);
        
        // Display result
        printf("Result: %.6g\n", result);
        
        // Add to history
        calculator_add_to_history(calc, input, result);
    }
}

// Main function
int main(int argc, char** argv) {
    printf("RaeenOS Calculator v1.0\n");
    printf("A simple calculator for RaeenOS\n\n");
    
    calculator_t calc;
    calculator_init(&calc);
    
    // Display welcome screen
    calculator_display(&calc);
    
    // Run calculator
    calculator_run(&calc);
    
    return 0;
} 