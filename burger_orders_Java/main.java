import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Scanner;

/**
 *  Process burger orders, 4 languages. Java.
 *  Tested with java 1.7
 */

/**
 * Main class and implementation
 */
class Main {

	public static class OneBurger {
		public String bun_type;
		public List<String> addit_fillings = new ArrayList<String>();
		public double sum = 0.;

		OneBurger(String abun_ty, List<String> aaddit_fillings) {
			bun_type = abun_ty;
			addit_fillings = aaddit_fillings;
			sum = 0.;
		}
	}

	public static class BurgerOrder {
		public List<OneBurger> burgers_list = new ArrayList<OneBurger>();
		public String delivery_type;
		// client's: name, phone_number and address
		public String name = "";
		public String phone_number = "";
		public String address = "";
		public double sum = 0.;

		BurgerOrder(List<OneBurger> aburgers_list, String adelivery_type) {
			burgers_list = aburgers_list;
			delivery_type = adelivery_type;
			// client's: name, phone_number and address
			name = "";
			phone_number = "";
			address = "";
			sum = 0.;
		}
	}

	// *** Cost Calculation part ***

	// "Standard buns cost $2, multigrain buns cost $3 and brioche buns cost $4."
	static Map<String, Double> bun_costs = new HashMap<String, Double>() {
		{
			put("standard", 2.0);
			put("multigrain", 3.0);
			put("brioche", 4.0);
		}
	};

	static double get_bun_cost(String bun_type) {
		if (!bun_costs.containsKey(bun_type)) {
			throw new RuntimeException("Bun type unknown: " + bun_type);
		}
		return bun_costs.get(bun_type);
	}

	static Map<String, Double> filling_costs = new HashMap<String, Double>() {
		{
			put("beef pattie", 1.0); // (so you can have multiple patties in a
										// burger),
			put("bacon", 1.0);
			put("lettuce", 1.0);
			put("mushrooms", 1.0);
			put("pineapple", 1.0);
			put("jalapenos", 1.0);
		}
	};

	static double get_filling_cost(String filling) {
		if (!filling_costs.containsKey(filling)) {
			throw new RuntimeException("Filling unknown: " + filling);
		}
		return filling_costs.get(filling);
	}

	static double calc_one_burger_order(OneBurger one_burger_order1) {
		Integer n = one_burger_order1.addit_fillings.size();
		if (n > 5)
			throw new RuntimeException("Maximum 5 fillings are allowed, "
					+ n.toString() + "found fillings");
		double sum = 0.;
		// All burgers come with a Beef pattie by default, at no extra cost.
		sum += get_bun_cost(one_burger_order1.bun_type);
		for (String filling : one_burger_order1.addit_fillings)
			sum += get_filling_cost(filling);
		return sum;
	}

	static double calc_burger_order_total(BurgerOrder burger_order1) {
		double sum = 0.;
		for (OneBurger one_burger_order1 : burger_order1.burgers_list)
			sum += calc_one_burger_order(one_burger_order1);
		if (burger_order1.delivery_type.equals("deliver") && sum < 30.0)
			sum += 8.0; // add delivery fee
		burger_order1.sum = sum;
		return sum;
	}

	// *** Data Entry and User input processing part part ***

	// Database Keeps all orders
	static List<BurgerOrder> database = new ArrayList<BurgerOrder>();

	// table for normalized names: maps type;option-selection-key string
	// ->into-> normalized name
	static Map<String, String> norm_names = new HashMap<String, String>() {
		{
			// bun type:
			put("bun;s", "standard");
			put("bun;m", "multigrain");
			put("bun;b", "brioche");

			put("filling;b", "beef pattie");
			put("filling;c", "bacon");
			put("filling;l", "lettuce");
			put("filling;m", "mushrooms");
			put("filling;p", "pineapple");
			put("filling;j", "jalapenos");

			put("delivery;c", "collect");
			put("delivery;d", "deliver");
		}
	};

	public static class Tuple<X, Y> {
		public final X x;
		public final Y y;

		public Tuple(X x, Y y) {
			this.x = x;
			this.y = y;
		}
	}

	/*
	 * convert input to normalized form type is one of: bun type: "standard":
	 * 2.0, "multigrain": 3.0, "brioche": 4.0}
	 */
	public static Tuple<String, String> normalize_input(String atype,
			String inputstr) {
		String type = atype.toLowerCase();
		String inputstrLow = inputstr.toLowerCase();
		String key = type + ";" + inputstrLow;
		String sErr, norm_name;
		if (!norm_names.containsKey(key)) {
			sErr = String.format("No such thing '%s' in %s types", inputstr,
					type);
			norm_name = "";
		} else {
			sErr = "";
			norm_name = norm_names.get(key);
		}
		return new Tuple<String, String>(sErr, norm_name);
	}

	/**
	 * Function from Internet:
	 * http://stackoverflow.com/questions/1751844/java-convert
	 * -liststring-to-a-joined-string BTW now available in Java 8
	 */
	public static String String_join(String delim, Collection<?> col) {
		StringBuilder sb = new StringBuilder();
		Iterator<?> iter = col.iterator();
		if (iter.hasNext())
			sb.append(iter.next().toString());
		while (iter.hasNext()) {
			sb.append(delim);
			sb.append(iter.next().toString());
		}
		return sb.toString();
	}

	/** generate beautiful options from database names */
	static String generate_menu(String type) {
		// "filling"
		List<String> menu_list = new ArrayList<String>();
		int tslen = type.length(); // type str len
		// for k,v in norm_names.items():
		for (Map.Entry<String, String> entry : norm_names.entrySet()) {
			String k = entry.getKey();
			String v = entry.getValue();
			if (   k.length()>=tslen 
				&& k.substring(0, tslen).equals(type)) {
				String menu_option = String.format("(%s)%s", v.substring(0, 1)
						.toUpperCase(), v.substring(1));
				menu_list.add(menu_option);
			}
		}
		// create menu from menu_list
		String res = String_join(", ", menu_list);
		return res;
	}

	static String user_input_from_limited_options(String stype,
			String prompt_str) {
		return user_input_from_limited_options_custom(stype, prompt_str, "", "");
	}

	static String user_input_from_limited_options_custom(String stype,
			String prompt_str, String custom_menu, String custom_exit_key) {
		String normalized_option, menu, key_for_option, sErr;
		while (true) {
			if (custom_menu.equals(""))
				menu = generate_menu(stype);
			else
				menu = custom_menu;

			key_for_option = raw_input(prompt_str + ": " + menu + ": ");
			if (!custom_menu.equals(""))
				if (key_for_option.toLowerCase().equals(custom_exit_key))
					return custom_exit_key;
			Tuple<String, String> t = normalize_input(stype, key_for_option);
			sErr = t.x;
			normalized_option = t.y;
			if (!sErr.equals("")) {
				System.out.println(sErr);
				continue;
			}
			// Entry was OK! exiting
			break;
		}
		return normalized_option;
	}

	// Java 8 has lambdas... ; but here we use java7 so using interfaces
	public interface Validator {
		public boolean call(String data);
	}

	public static class validator_len3 implements Validator {
		public boolean call(String s) {
			return s.length() >= 3;
		}
	}

	public static class validator_len5 implements Validator {
		public boolean call(String s) {
			return s.length() >= 5;
		}
	}

	public static class validator_len5_alldigits implements Validator {
		public boolean call(String s) {
			return s.length() >= 5 && s.matches("[0-9]+");
		}
	}

	public static class validator_any implements Validator {
		public boolean call(String s) {
			return true;
		}
	}

	static String user_input_with_validator(String prompt_str,
			Validator validate_func) {
		String user_input = "";
		while (true) {
			user_input = raw_input(prompt_str + ": ");
			boolean ok = validate_func.call(user_input);
			if (!ok) {
				System.out.println(" Wrong input, please try again");
				continue;
			}
			// Entry was OK! exiting
			break;
		}
		return user_input;
	}

	static void enter_new_order() {
		ArrayList<OneBurger> emptyListB = new ArrayList<OneBurger>();
		BurgerOrder bo = new BurgerOrder(emptyListB, "none_delivtype");

		while (true) {
			String new_burger_msg = "Enter N(ew) and <Enter> (or any other key) to add new burger into order, otherwise press just <Enter> when you done:";
			String newQ = raw_input(new_burger_msg);
			if (newQ.equals(""))
				// finish entering in the burgers
				break;

			ArrayList<String> emptyListS = new ArrayList<String>();
			OneBurger ob = new OneBurger("none_buntype", emptyListS);

			// Ask: bun type
			String norm_bun_type = user_input_from_limited_options("bun",
					"Enter bun type of burger");
			ob.bun_type = norm_bun_type;

			// Ask: fillings
			int i = 0;
			while (i < 5) {
				// menu = generate_menu("filling")
				// Set here hand-coded value for menu, since generated doesn't
				// do well in first letter collision case:
				String prompt_f = "Enter additional filling for burger";
				String menu_f = "(B)eef pattie, ba(C)con, (M)ushrooms, (L)ettuce, (J)alapenos, (P)ineapple";
				String sExit_msg = "\n Or press 'e' for E(nough) of additional fillings";

				String norm_filling = user_input_from_limited_options_custom(
						"filling", prompt_f, menu_f + sExit_msg, "e");
				if (norm_filling.equals("e"))
					break;
				// Add filling to the burger
				ob.addit_fillings.add(norm_filling);
				System.out.format("%s filling added\n", norm_filling);
				i += 1;
			}
			System.out.format(
					"Finished with fillings: %d additional fillings added.\n",
					ob.addit_fillings.size());

			bo.burgers_list.add(ob);
			System.out.format("\nBurger added to the order: price $%.2f\n\n",
					calc_one_burger_order(ob));

			if (bo.burgers_list.size() > 0) {
				// Ask: delivery type
				String norm_delivery_type = user_input_from_limited_options(
						"delivery", "Enter delivery type of order");
				bo.delivery_type = norm_delivery_type;

				// Ask: name
				// bo.name = raw_input("Enter customer's name:")
				bo.name = user_input_with_validator(
						"Enter customer's name for order (min 3 chars)",
						new validator_len3());
				// Ask: phone
				bo.phone_number = user_input_with_validator(
						"Enter customer's phone number (digits only, min 5 chars)",
						new validator_len5_alldigits());

				// Ask: address
				// Choose right validator depending of delivery type:
				Validator validatorNow = bo.delivery_type.equals("deliver") ? new validator_len5()
						: new validator_any();
				bo.address = user_input_with_validator(
						"Enter customer's address (min 5 chars) (optional in case of collected order)",
						validatorNow);
				// end of Burger order entry! ==

				System.out.format("\nburger order: %d burger(s)\n",
						bo.burgers_list.size());
				System.out.format("burger order total: $%.2f\n\n",
						calc_burger_order_total(bo));
				database.add(bo);
			}
		}

		System.out.format("\n%d total order(s) in database at the moment.\n\n",
				database.size());
	}

	public static void print_one_burger_details(OneBurger b) {
		System.out.println("Burger with:");
		System.out.println(" bun: " + b.bun_type);
		String s_addit_fillings = (b.addit_fillings.size() > 0) ? String_join(
				", ", b.addit_fillings) : "none";
		System.out.println(" additional filling(s): " + s_addit_fillings);
	}

	/**
	 * burger_order1 = The details of each burger in the order, The total cost
	 * of the order, and The name, phone number and (if required) address of the
	 * person who made the order.
	 */
	public static void print_order_details(BurgerOrder bo) {

		System.out.format("Order with %d burger(s):\n", bo.burgers_list.size());
		for (OneBurger b : bo.burgers_list)
			print_one_burger_details(b);
		
		// Customer's name:
		System.out.println("Customer's name: "+bo.name);
		// Customer's phone number:
		System.out.println("Customer's phone number: "+bo.phone_number);
		// Customer's address:
		System.out.println( "Customer's address: "+bo.address);

		System.out.println(" End of order\n");
	}

	public static void print_database() {
		System.out.format("All orders list: %d item(s)\n", database.size());
		for (BurgerOrder bo1 : database)
			print_order_details(bo1);
		System.out.println("End of orders database\n\n");
	}

	// ***

	static Scanner input = new Scanner(System.in);

	static String raw_input(String prompt) {
		System.out.print(prompt);
		String answer = input.nextLine();
		return answer;
	}

	static int main_user_input() {
		/**
		 * Main operation cycle. ask operation: enter a new order show all
		 * orders mark selected order as done (delivered or collected and money
		 * taken)
		 */
		while (true) {
			// print menu:
			String menu = "Choose the operation, please:\n"
					+ " E(nter new order)\n" + " S(how all orders)\n"
					+ " Q(uit)  /or empty/\n";

			// Read operation:
			String cmd = raw_input(menu);
			if (cmd.equals(""))
				cmd = "q";
			cmd = cmd.substring(0, 1).toLowerCase();

			if (cmd.equals("q"))
				break; // and quit
			else if (cmd.equals("e"))
				enter_new_order();
			else if (cmd.equals("s"))
				print_database();
			else
				System.out.format("\nWrong operation '%s' received, please try again.\n", cmd);
		}
		System.out.println("Done. Quitting.");
		return 0;
	}

	// ===
	boolean test() {
		main_user_input();
		return true;
	}

	public static void main(String[] args) {
		// test();
		main_user_input();
	}

}
