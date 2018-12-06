#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //for true and false
#include <string.h>
#include "../ccl/containers.h"
/*
List of used types:mapping types from Java to C Container library (CCL):

ArrayList<String> -> strCollection / istrCollection
  iterator
ArrayList<OneBurger> -> Vector/iVector
  ;
String  -> char* + strdup()+ free()

HashMap<String, Double> -> Dictionary/iDictionary
HashMap<String, String>  -""
  containsKey(),get() -> getElement()

ArrayList<BurgerOrder> -> Vector //database =
*/

void print_dict(Dictionary* dict, char* dname);

static void ABORT(char* file,int line)
{
    fprintf(stderr,"*****\n\nABORT\nFile %s Line %d\n**********\n\n",file,line);
    abort();
}
#define Abort() ABORT(__FILE__,__LINE__)
void error(char* msg){
  fprintf(stderr, "%s\n", msg);
  Abort();
}

//
struct OneBurger
{
    char*    bun_type; //String
    strCollection* addit_fillings; //ArrayList<String>
    double   sum;
};

struct OneBurger* ctor_OneBurger(const char* abun_ty) {  //-, strCollection* aaddit_fillings
    struct OneBurger* this = malloc(sizeof (struct OneBurger));

    this->bun_type = strdup(abun_ty);
    //-this->addit_fillings = aaddit_fillings; //copy
    this->addit_fillings = istrCollection.Create(6); //6 is good number, resize will not be needed
    this->sum = 0.;

    return this;
}

struct BurgerOrder {
    Vector* burgers_list; // ArrayList<OneBurger>
    char*   delivery_type;
    // client's: name, phone_number and address
    char*   name;
    char*   phone_number;
    char*   address;
    double  sum;
};

struct BurgerOrder* ctor_BurgerOrder(const char* adelivery_type) { //-Vector* aburgers_list,
    struct BurgerOrder* this = malloc(sizeof(struct BurgerOrder));
    //0this.burgers_list = aburgers_list; //-copy
    this->burgers_list =  iVector.Create(sizeof(struct OneBurger), 10); // !not sizeof(struct OneBurger*)
    this->delivery_type = strdup( adelivery_type);
    // client's: name, phone_number and address
    this->name = NULL; //strdup("");
    this->phone_number = NULL;
    this->address = NULL;
    this->sum = 0.;

    return this;
}

// *** Cost Calculation part ***

// "Standard buns cost $2, multigrain buns cost $3 and brioche buns cost $4."
Dictionary* bun_costs; //HashMap<String, Double>

void ctor_bun_costs()
{
    bun_costs= iDictionary.Create(sizeof(double), 3);

    static double bcosts[3] = {2.0, 3.0, 4.0};

    iDictionary.Add(bun_costs, "standard",   &bcosts[0]);
    iDictionary.Add(bun_costs, "multigrain", &bcosts[1]);
    iDictionary.Add(bun_costs, "brioche",    &bcosts[2]);
}

double get_bun_cost(const char* bun_type) {
    //if (!bun_costs.containsKey(bun_type)) {
    double* pd = (double*)iDictionary.GetElement(bun_costs, bun_type);
    if(pd==NULL) {
        //throw new RuntimeException
        //iError.RaiseError(,,,);
        fprintf(stderr, "Bun type unknown: %s\n", bun_type);
        return 0.;
    }
    return *pd;
}

Dictionary* filling_costs; //HashMap<String, Double>
void ctor_filling_costs()
{
    filling_costs = iDictionary.Create(sizeof(double), 6);
    static double fcosts[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

    iDictionary.Add(filling_costs, "beef pattie",&fcosts[0]); // (so you can have multiple patties in a burger)
    iDictionary.Add(filling_costs, "bacon",      &fcosts[1]);
    iDictionary.Add(filling_costs, "lettuce",    &fcosts[2]);
    iDictionary.Add(filling_costs, "mushrooms",  &fcosts[3]);
    iDictionary.Add(filling_costs, "pineapple",  &fcosts[4]);
    iDictionary.Add(filling_costs, "jalapenos",  &fcosts[5]);

}

double get_filling_cost(const char* filling) {
    double* pd = (double*)iDictionary.GetElement(filling_costs, filling);
    if(pd==NULL) {
        fprintf(stderr, "Filling unknown: %s\n", filling);
        return 0.;
    }
    return *pd;
}

double calc_one_burger_order(struct OneBurger* pob) {
    size_t n = istrCollection.Size( pob->addit_fillings);
    if (n > 5){
        //throw new RuntimeException
        fprintf(stderr, "Maximum 5 fillings are allowed, found %d fillings", n);
    }
    double sum = 0.;
    // All burgers come with a Beef pattie by default, at no extra cost.
    sum += get_bun_cost(pob->bun_type);

    //for (String filling : pone_burger_order1->addit_fillings)
    Iterator* it = istrCollection.NewIterator(pob->addit_fillings);
    char* f; //filling
    for ( f = it->GetFirst(it);
          f!= NULL;
          f = it->GetNext(it)
        )
        sum += get_filling_cost(f);
    istrCollection.DeleteIterator(it);

    pob->sum = sum;

    return sum;
}

double calc_burger_order_total(struct BurgerOrder* pburger_order1) {
    double sum = 0.;

    //for (OneBurger* pone_burger_order1 : pburger_order1->burgers_list)
    //    sum += calc_one_burger_order(pone_burger_order1);

    Iterator* it = iVector.NewIterator(pburger_order1->burgers_list);
    struct OneBurger* pob;
    for ( pob = it->GetFirst(it);
          pob!= NULL;
          pob = it->GetNext(it)
        )
        sum += calc_one_burger_order(pob);
    iVector.DeleteIterator(it);

    if (    0==strcmp(pburger_order1->delivery_type, "deliver")
         && sum < 30.0 )
        sum += 8.0; // add delivery fee
    pburger_order1->sum = sum;
    return sum;
}


// *** Data Entry and User input processing part part ***

// Database : Keeps all orders
Vector* database; //ArrayList<BurgerOrder>
void ctor_database() {
  database = iVector.Create(sizeof(struct BurgerOrder), 10); //BurgerOrder*
}

// table for normalized names: maps type;option-selection-key string
// ->into-> normalized name
Dictionary* norm_names; //HashMap<String, String>
void ctor_norm_names() {
    // 1. ElementSize here == 12bytes. this is value string max length
    //   was: sizeof(char*)==8 => copies only 8 chars on x86_64; and will copy 4 chars on x86/32bit
    // 2. 11 = 3+6+2 == hint for size = exact elements count here
    norm_names = iDictionary.Create(sizeof(char*), 11); //12
    //
    char* data[]= { "standard", "multigrain", "brioche",
                    "beef pattie", "bacon", "lettuce",
                     "mushrooms", "pineapple", "jalapenos",
                    "collect", "deliver"
                  };
    //
    iDictionary.Add(norm_names, "bun;s",      &data[0]); //"standard");
    iDictionary.Add(norm_names, "bun;m",      &data[1]); //"multigrain");
    iDictionary.Add(norm_names, "bun;b",      &data[2]); //"brioche");

    iDictionary.Add(norm_names, "filling;b",  &data[3]); //"beef pattie");
    iDictionary.Add(norm_names, "filling;c",  &data[4]); //"bacon");
    iDictionary.Add(norm_names, "filling;l",  &data[5]); //"lettuce");
    iDictionary.Add(norm_names, "filling;m",  &data[6]); //"mushrooms");
    iDictionary.Add(norm_names, "filling;p",  &data[7]); //"pineapple");
    iDictionary.Add(norm_names, "filling;j",  &data[8]); //"jalapenos");

    iDictionary.Add(norm_names, "delivery;c", &data[9]); //"collect");
    iDictionary.Add(norm_names, "delivery;d", &data[10]); //"deliver");

    //print_dict( norm_names, "norm_names");
}

struct TupleStrStr {
    char* x;
    char* y;
};

struct TupleStrStr* ctor_TupleStrStr(char* x, char* y) {
    struct TupleStrStr* this = malloc(sizeof(struct TupleStrStr));
    if (this==0)
      return 0;
    this->x = strdup(x);
    this->y = strdup(y);
    return this;
}

void dtor_TupleStrStr(struct TupleStrStr* this) {
  free(this->x);
  free(this->y);
  free(this);
}
/// Small strings library ///

//const unsigned int
#define BUFFERSIZE 256

char* raw_input(const char* menu) {
  static char buffer[BUFFERSIZE];
  printf(menu);
  char* p = fgets(buffer, sizeof(buffer), stdin);
  int len = strlen(p);
  //eliminate carriage return char(s)
  if (len>0 && p!=0)
    p[len-1]='\0';
  return p;
}

/** tolower for string: performs in-place replacement */
char* strlower(char* s) {
  size_t i;
  if(!s)
    return s;
  size_t len = strlen(s);
  if (len==0)
    return s;
  for(i=0; i<len; ++i)
    s[i] = (char) tolower(s[i]);
  return s;
}

/** toupper for string: performs in-place replacement */
char* strupper(char* s) {
  size_t i;
  if(!s)
    return s;
  size_t len = strlen(s);
  if (len==0)
    return s;
  for(i=0; i<len; ++i)
    s[i] = (char) toupper(s[i]);
  return s;
}

/** Creates news string, filled in with substring of src, from start count chars.
    @param count when count, as int is <=0 ==(size_t)-1  this means "copy until the end of string - count",
     i.e. count from the end of string.
    @return new string
 */
char* substring(char* src, size_t start, int count)
{
  if (src==0)
    return 0;
  if (0==strlen(src))
    return src;
  if( count<=0)
    count = strlen(src) - start - count;

  char* resstr = malloc((size_t)count+1);
  memcpy(resstr, src+start, (size_t)count);
  resstr[count]=0;
  return resstr;
}

// Unused:
//  //see file concatn.c
// extern char* concatn(int count, ...);

// end of strings mini library

/*
 * convert input to normalized form type is one of:
 *  bun type: "standard": 2.0, "multigrain": 3.0, "brioche": 4.0}
 */
struct TupleStrStr* normalize_input(char* atype, char* inputstr) {
    char* type = strlower(strdup(atype));
    char* inputstrLow = strlower(strdup(inputstr));
    //char* key = type '+' ";" '+' inputstrLow;
    char key[100];
    snprintf(key, sizeof(key), "%s;%s", type, inputstrLow);

    char* sErr;
    char* norm_name;
    char** pnorm_name = iDictionary.GetElement(norm_names, key);
    if (0 ==pnorm_name) {
        sErr = malloc(100);
        sprintf(sErr, "No such thing '%s' in %s types", inputstr, type);
        norm_name = "";
    } else {
        sErr = "";
        norm_name = *pnorm_name;
    }

    free(type);
    free(inputstrLow);
    return ctor_TupleStrStr(sErr, norm_name);
}

/**
 * Ported Java function from Internet to CCL:
 * orig. Java: http://stackoverflow.com/questions/1751844/java-convert-liststring-to-a-joined-string
 * BTW String.join() is available in Java 8
 */
char* String_join(char* delim, strCollection* col) {
    static char joined[1024];
    joined[0]=0;

    Iterator* it = istrCollection.NewIterator(col);
    char* s = it->GetFirst(it);
    if ( s ) {
       strcpy(joined, s);
       s = it->GetNext(it);
    }
    for ( ; s!= NULL; s = it->GetNext(it) ) {
        strcat(joined, delim);
        strcat(joined, s);
    }
    istrCollection.DeleteIterator(it);
    return strdup(joined);
}

/** //Iterate dictionary by values:
Iterator* it = iDictionary.NewIterator(norm_names);
char* pval;
for (pval = it->GetFirst(it); pval != NULL; pval = it->GetNext(it)) {
    // Work with the value here:
    printf(" val=%s\n", pval);
}
iDictionary.DeleteIterator(it);
*/


/** generate beautiful options from database names
 */
char* generate_menu(char* type) {
    // menu_list  will hold menu option strings
    strCollection* menu_list = istrCollection.Create(6); //ArrayList<String>
    int tslen = strlen(type); // type str len
    // for k,v in norm_names.items():

    // Iterate dictionary by keys -> and by such way by <key:value> pairs :
    strCollection* keys_arr = iDictionary.GetKeys(norm_names);
    Iterator* it = istrCollection.NewIterator(keys_arr);
    char* dkey; // dictionary key
    char menu_option[256];
    char *s1, *s2,*s3; //vars ret by substring: to free()
    //printf(" =====\n");

    for ( dkey = it->GetFirst(it);
          dkey!= NULL;
          dkey = it->GetNext(it)
        )
    {
        char** pval = iDictionary.GetElement(norm_names, dkey);
        if(pval==0)
          error("Runtime error with dict. key");
        char* val = *pval;
        //printf(" key=%s val=%s\n", dkey, val);

        if (     strlen(dkey) >=tslen
              && 0==strcmp( s1=substring(dkey, 0, tslen), type)
           )
        {
            sprintf(menu_option , "(%s)%s", strupper(s2=substring(val, 0, 1)),  s3=substring(val, 1, 0) );
            istrCollection.Add(menu_list, menu_option);
            free(s2);
            free(s3);
            free(s1); //s1 was allocated only if is passed here, into the body of this 'if'
        }
    }
    istrCollection.DeleteIterator(it);

    // create menu from menu_list
    char* res = String_join(", ", menu_list);
    return res;
}

/**
  in custom call case, i.e. when  custom_menu and custom_exit_key are not "",
     returns on exit custom_exit_key value, so it can be reliably compared.
 */
char* user_input_from_limited_options_custom(char* stype, char* prompt_str, char* custom_menu, char* custom_exit_key) {
    char* normalized_option;
    char* menu;
    char* key_for_option;
    char* sErr;
    while (true) {
        if (0 == strcmp(custom_menu, ""))
            menu = generate_menu(stype);
        else
            menu = custom_menu;

        char menu1[256];
        sprintf(menu1, "%s%s%s%s", prompt_str, ": ", menu, ": ");
        key_for_option = raw_input(menu1);

        if (0!=strcmp(custom_menu, ""))
            if ( 0 == strcmp( strlower(key_for_option), custom_exit_key))
                return custom_exit_key;
        struct TupleStrStr* pt = normalize_input(stype, key_for_option);
        sErr = pt->x;
        normalized_option = strdup(pt->y); //since it will be returned
        if ( 0!= strcmp(sErr, "")) {
            fprintf(stderr, "%s\n", sErr);
            free(pt);
            continue;
        }
        free(pt);
        // Entry was OK! exiting
        break;
    }
    return normalized_option;
}

char* user_input_from_limited_options(char* stype, char* prompt_str) {
    return user_input_from_limited_options_custom(stype, prompt_str, "", "");
}


// Validator func :
typedef bool (*PValidator)(char* data);

char* user_input_with_validator(char* prompt_str, PValidator pvalidate_func) {
    char* user_input = "";
    while (true) {
        char prmt2[256];
        sprintf(prmt2, "%s%s", prompt_str, ": ");

        user_input = strdup( raw_input(prmt2) );
        bool ok = (*pvalidate_func)(user_input);
        if (!ok) {
            printf(" Wrong input, please try again\n");
            continue;
        }
        // Entry was OK! exiting
        break;
    }
    return user_input;
}

bool validator_len3(char* s) {
    return strlen(s) >= 3;
}

bool validator_len5(char* s) {
    return strlen(s) >= 5;
}

bool validator_len5_alldigits(char* s) {
    bool res;
    size_t len = strlen(s);
    res= len >= 5;
    if (!res)
      return false;
    //s.matches("[0-9]+" :
    for(size_t i=0; i<len; ++i)
      if(!isdigit(s[i])) {
        res=false;
        break;
      }
    return res;
}

bool validator_any(char* s) {
    return true;
}

/** */
void enter_new_order() {
    struct BurgerOrder* bo = ctor_BurgerOrder("none_delivtype");

    while (true) {
        char* new_burger_msg = "Enter N(ew) and <Enter> (or any other key) to add new burger into order, "
                               "otherwise press just <Enter> when you done: ";
        char* newQ = raw_input(new_burger_msg);
        if (0== strcmp(newQ, ""))
            // finish entering in the burgers
            break;

        //-strCollection* emptyListS = istrCollection.Create(6); //ArrayList<String>
        struct OneBurger* ob = ctor_OneBurger("none_buntype");

        // Ask: bun type
        char* norm_bun_type = user_input_from_limited_options("bun", "Enter bun type of burger");
        ob->bun_type = norm_bun_type;

        // Ask: fillings
        int i = 0;
        while (i < 5) {
            // menu = generate_menu("filling")
            // Set here hand-coded value for menu, since generated doesn't
            // do well in first letter collision case:
            char* prompt_f = "Enter additional filling for burger";
            char* menu_f_sExit_msg =
              "(B)eef pattie, ba(C)con, (M)ushrooms, (L)ettuce, (J)alapenos, (P)ineapple"
              "\n Or press 'e' for E(nough) of additional fillings";

            char* norm_filling = user_input_from_limited_options_custom(
                    "filling", prompt_f, menu_f_sExit_msg, "e");
            if (0 ==strcmp(norm_filling, "e"))
                break;
            // Add filling to the burger
            istrCollection.Add(ob->addit_fillings, norm_filling); //...
            printf("%s filling added\n", norm_filling);
            i += 1;
        }
        printf("Finished with fillings: %d additional fillings added.\n",
                 istrCollection.Size( ob->addit_fillings));
        //burger is ready: print it:
        calc_one_burger_order(ob);
        print_one_burger_details(ob);

        iVector.Add(bo->burgers_list, ob);
        printf("\nBurger added to the order: price $%.2f\n\n",  calc_one_burger_order(ob));

        if ( iVector.Size(bo->burgers_list) > 0) {
            // Ask: delivery type
            char* norm_delivery_type = user_input_from_limited_options(
                    "delivery", "Enter delivery type of order");
            bo->delivery_type = norm_delivery_type;

            // Ask: name
            // bo.name = raw_input("Enter customer's name:")
            bo->name = user_input_with_validator(
                    "Enter customer's name for order (min 3 chars)",
                    validator_len3);
            // Ask: phone
            bo->phone_number = user_input_with_validator(
                    "Enter customer's phone number (digits only, min 5 chars)",
                    validator_len5_alldigits);

            // Ask: address
            // Choose right validator depending of delivery type:
            PValidator validatorNow = (0==strcmp(bo->delivery_type, "deliver")) ? validator_len5 : validator_any;
            bo->address = user_input_with_validator(
                    "Enter customer's address (min 5 chars) (optional in case of collected order)",
                    validatorNow);
            // end of Burger order entry! ==

            printf("\nburger order: %d burger(s)\n", iVector.Size( bo->burgers_list));
            printf("burger order total: $%.2f\n\n", calc_burger_order_total(bo));
            iVector.Add(database, bo);
        }
    }

    printf("\n%d total order(s) in database at the moment.\n\n", iVector.Size( database));
}
//

void print_one_burger_details(struct OneBurger* pb) {
    printf("Burger with:\n");
    printf(" bun: %s\n", pb->bun_type);
    char* s_addit_fillings = (istrCollection.Size(pb->addit_fillings) > 0)
       ? String_join(", ", pb->addit_fillings)
       : "none";
    printf(" additional filling(s): %s\n", s_addit_fillings);
    printf(" sum: $%.2lf\n", pb->sum);
}

/**
 * pbo = The details of each burger in the order, The total cost
 * of the order, and The name, phone number and (if required) address of the
 * person who made the order.
 */
void print_order_details(struct BurgerOrder* pbo) {
    printf("Order with %d burger(s):\n", iVector.Size(pbo->burgers_list));
    //for (OneBurger* pb : pbo->burgers_list)
    Iterator* it = iVector.NewIterator(pbo->burgers_list);
    struct OneBurger* pob;
    for(pob = it->GetFirst(it); pob!= NULL; pob = it->GetNext(it))
        print_one_burger_details(pob);
    iVector.DeleteIterator(it);

    // Customer's name:
    printf("Customer's name: %s\n", pbo->name);
    // Customer's phone number:
    printf("Customer's phone number: %s\n", pbo->phone_number);
    // Customer's address:
    printf( "Customer's address: %s\n", pbo->address);

    printf( "Total: $%.2lf\n", pbo->sum);

    printf(" End of order\n\n");
}

void print_database() {
    printf("All orders list: %d item(s)\n", iVector.Size( database) );
    //for (BurgerOrder* pbo1 : database)
    Iterator* it = iVector.NewIterator(database);
    struct BurgerOrder* pbo;
    for (pbo= it->GetFirst(it); pbo !=NULL; pbo = it->GetNext(it))
       print_order_details(pbo);
    iVector.DeleteIterator(it);
    printf("End of orders database\n\n");
}

/**
 * Main operation cycle. ask operation: enter a new order show all
 * orders mark selected order as done (delivered or collected and money
 * taken)
 */
int main_user_input() {

    while (true) {
        // print menu:
        const char* menu = "Choose the operation, please:\n"
                 " E(nter new order)\n"
                 " S(how all orders)\n"
                 " Q(uit)  /or empty/\n";

        // Read operation:
        char* cmd = raw_input(menu);
        if (0==strcmp(cmd, "") )
            cmd = "q";
        cmd = strlower(cmd);

        if (0==strcmp(cmd, "q"))
            break; // and quit
        else if (0==strcmp(cmd, "e"))
            enter_new_order();
        else if (0==strcmp(cmd, "s"))
            print_database();
        else
            printf("\nWrong operation '%s' received, please try again.\n", cmd);
    }
    printf("Done. Quitting.");
    return 0;
}
/** */

void print_dict(Dictionary* dict, char* dname) {
    printf("print-dict %s=\n", dname);
    strCollection* keys_arr = iDictionary.GetKeys(dict);
    Iterator* it = istrCollection.NewIterator(keys_arr);
    char* dkey; // dictionary key
    char menu_option[256];
    //char *s1, *s2,*s3; //vars ret by substring: to free()

    for ( dkey = it->GetFirst(it);
          dkey!= NULL;
          dkey = it->GetNext(it)
        )
    {
        char** pval = iDictionary.GetElement(dict, dkey);
        if(pval==0)
          error("Runtime error with dict. key");
        printf(" key=%s val=%s\n", dkey, *pval);
    }
    istrCollection.DeleteIterator(it);
    printf("  print_dict done.\n");
}

bool test2_containers() {
    printf("test2():\n");
    //construct:
    // one burger:
    struct OneBurger* ob = ctor_OneBurger("none_buntype");
    printf(" addit_fillings to_be=0x%x\n", (int)ob->addit_fillings);

    ob->bun_type = "standard";
    ob->sum = 3.14;

    // fillings list

    char* norm_filling = "mushrooms";
    istrCollection.Add(ob->addit_fillings, norm_filling); //, &norm_filling);<<!!

    char* norm_filling2 = "beef pattie";
    istrCollection.Add(ob->addit_fillings, norm_filling2); //, &norm_filling);<<!!


    printf("Finished with fillings: %d additional fillings added.\n",
         istrCollection.Size( ob->addit_fillings));
    //to set the price:
    calc_one_burger_order(ob);
    // one burger

     //burger is ready: print it:
     print_one_burger_details(ob);
     //i.e. burger now is OK!

    // burger order
    Vector* emptyListB = iVector.Create(sizeof(struct OneBurger), 10); //OneBurger*  //ArrayList<OneBurger>
    struct BurgerOrder* bo = ctor_BurgerOrder("none_delivtype"); //new BurgerOrder(emptyListB, "none_delivtype");
    ;
    int iRes = iVector.Add(bo->burgers_list, ob);
    printf("\nBurger added to the order: price $%.2f\n\n",  calc_one_burger_order(ob));

    struct OneBurger* obRetr = iVector.GetElement(bo->burgers_list, 0);
    printf("\nBurger RETR:\n");
    print_one_burger_details(obRetr);

    char* norm_delivery_type = "";
    bo->delivery_type = "collect"; //"deliver";
    bo->name = "Cust";
    bo->phone_number = "12345";
    bo->address = "str. Abc";

    // add to database
    printf("\nburger order: %d burger(s)\n", iVector.Size( bo->burgers_list));
    // and now at calc_burger_order_total()  burger is *corrupted*!!
      //precheck: is burger OK?
      print_one_burger_details(ob);
    print_order_details(bo);
    printf("burger order total: $%.2f\n\n", calc_burger_order_total(bo));
    iVector.Add(database, bo);

    //print:
    print_database();
    // cleanup :
    iVector.Clear(database);
    /** // "simple iteration" :
    for (size_t i=0; i<iContainer.Size(Container); i++) {
       someType *element = iContainer.GetElement(Container,i);
       // Use "element" here.
    }*/
    printf("end of test2\n");
    return true;
}

bool test() {
    printf("test():\n");
    printf(" bun: standard price=$%.2lf\n", get_bun_cost("standard"));
    printf(" filling: mushroom price=$%.2lf\n", get_filling_cost("mushrooms"));
    //
    printf("  norm_names.size()=%d\n", iDictionary.Size(norm_names) );
    char** pname= iDictionary.GetElement(norm_names, "delivery;c");
    if(0==pname)
      printf("   norm_names: IT IS NULL\n");
    else
      printf("   norm_names: %s\n", *pname);
    struct TupleStrStr* pT = normalize_input("bun", "B");
    printf(" normalize_input: %s %s\n", pT->x, pT->y);

    //
    print_dict( norm_names, "norm_names");
    //
    char* menu = generate_menu("filling");
    printf("  generate_menu=%s\n", menu);

    menu = generate_menu("bun");
    printf("  generate_menu=%s\n", menu);

    //
    test2_containers();

    //
    printf(" end of test\n\n");

    return true;
}

int main()
{
   //Call global constructors
   ctor_bun_costs();
   ctor_filling_costs();
   ctor_database();
   ctor_norm_names();
   //end of Global ctors

   //test(); //turn this on during testing phase
   main_user_input();

   return 0;
}
