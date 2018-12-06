#!/usr/bin/python
"""
 Process burger orders, 4 languages. Python.
 Tested with python 2.7
"""

class OneBurger:
	def __init__(self, bun_ty, addit_fillings):
		self.bun_type = bun_ty
		self.addit_fillings = addit_fillings
		self.sum = 0.

class BurgerOrder:
	def __init__(self, burgers_list, delivery_type):
		self.burgers_list = burgers_list
		self.delivery_type = delivery_type
		#client's: name, phone_number and address
		self.name = ""
		self.phone_number = ""
		self.address = ""

# === Cost Calculation part ===

#"Standard buns cost $2, multigrain buns cost $3 and brioche buns cost $4."
bun_costs = {"standard": 2.0, "multigrain": 3.0, "brioche": 4.0}
def get_bun_cost(bun_type):
	if not bun_costs.has_key(bun_type):
		raise Exception("Bun type unknown: "+bun_type)
		return 0
	return bun_costs[bun_type]

filling_costs = 	{ "beef pattie":1.0, # (so you can have multiple patties in a burger),
									  "bacon":1.0,
									  "lettuce":1.0,
									  "mushrooms":1.0,
									  "pineapple":1.0,
									  "jalapenos":1.0
                  }
def get_filling_cost(filling):
	if not filling_costs.has_key(filling):
		raise Exception("Filling unknown: "+filling)
		return 0
	return filling_costs[filling]

def calc_one_burger_order(one_burger_order1):
	n = len(one_burger_order1.addit_fillings)
	if n > 5:
		raise Exception("Maximum 5 fillings are allowed, found %d fillings" % n)
	sum = 0.
	#All burgers come with a Beef pattie by default, at no extra cost.
	sum+= get_bun_cost(one_burger_order1.bun_type)
	for filling in one_burger_order1.addit_fillings:
		sum+= get_filling_cost(filling)
	return sum

def calc_burger_order_total(burger_order1):
	sum = 0.
	for one_burger_order1 in burger_order1.burgers_list:
		sum += calc_one_burger_order( one_burger_order1)
	if burger_order1.delivery_type=="deliver" and sum<30.0:
		sum+= 8.0 # add delivery fee
	burger_order1.order_total = sum
	return sum

# === Data Entry and User input processing part part ===

#Database Keeps all orders
database = []

# table for normalized names: maps type;option-selection-key string ->into-> normalized name
norm_names={
	#bun type:
	"bun;s": "standard", "bun;m": "multigrain", "bun;b":"brioche",

	"filling;b":"beef pattie", "filling;c":"bacon",   "filling;l":"lettuce", "filling;m":"mushrooms",
  "filling;p":"pineapple",   "filling;j":"jalapenos",

	"delivery;c":"collect", "delivery;d":"deliver"
}

def normalize_input(type, inputstr):
	"""convert input to normalized form
	type is one of: bun type: "standard": 2.0, "multigrain": 3.0, "brioche": 4.0}
	"""
	type =type.lower()
	inputstrLow = inputstr.lower()
	key = type +";" + inputstrLow
	if not norm_names.has_key(key):
		sErr = "No such thing '%s' in %s types" %(inputstr, type)
		norm_name = ""
	else:
		sErr=""
		norm_name = norm_names[key]
	return sErr,norm_name

def generate_menu(type):
	"generate beautiful options from database names"
	#"filling"
	menu_list=[]
	tslen = len(type) # type str len
	for k,v in norm_names.items():
		if k[:tslen]==type:
			menu_option = "(%s)%s"%(v[0].upper(), v[1:])
			menu_list.append( menu_option)
	#create menu from menu_list
	res = ", ".join(menu_list)
	return res

def user_input_from_limited_options(stype, prompt_str, custom_menu=False, custom_exit_key=None):
	normalized_option=""
	while True:
		if not custom_menu:
			menu = generate_menu(stype)
		else:
			menu = custom_menu

		key_for_option = raw_input(prompt_str +", "+ menu + ": ")
		if custom_menu:
			if key_for_option.lower() == custom_exit_key:
				return custom_exit_key
		sErr, normalized_option = normalize_input(stype, key_for_option)
		if sErr:
			print sErr
			continue
		# Entry was OK! exitting
		break
	return normalized_option

def user_input_with_validator(prompt_str, validate_func):
	user_input = ""
	while True:
		user_input = raw_input(prompt_str+": ")
		ok = validate_func(user_input)
		if not ok:
			print " Wrong input, please try again"
			continue
		# Entry was OK! exitting
		break
	return user_input

def enter_new_order():
	bo = BurgerOrder([], "none_delivtype")

	while True:
		new_burger_msg = "Enter N(ew) and <Enter> (or any other key) to add new burger into order, otherwise press just <Enter> when you done:"
		newQ = raw_input(new_burger_msg)
		if newQ.lower()=="":
			#finish entry of burgers
			break

		ob = OneBurger("none_buntype",[])

		#Ask: bun type
		norm_bun_type = user_input_from_limited_options("bun", "Enter bun type of burger")
		ob.bun_type = norm_bun_type

		#Ask: fillings
		i=0
		while i<5:
			#menu = generate_menu("filling")
			# Set here hand-coded value for menu, since generated doesn't do well in first letter collision case:
			prompt_f = "Enter additional filling for burger: "
			menu_f = "(B)eef pattie, ba(C)con, (M)ushrooms, (L)ettuce, (J)alapenos, (P)ineapple"
			sExit_msg = "\n Or press 'e' for E(nough) of additional fillings"

			norm_filling = user_input_from_limited_options("filling", prompt_f, menu_f + sExit_msg, "e")
			if norm_filling=="e":
				break
			# Add filling to the burger
			ob.addit_fillings.append(norm_filling)
			print "%s filling added" % norm_filling
			i+=1
		print "Finished with fillings: %d additional fillings added." %( len(ob.addit_fillings))

		bo.burgers_list.append(ob)
		print "\nBurger added to the order: price $%.2f" %calc_one_burger_order(ob)
		print
	if len(bo.burgers_list)>0:
		#Ask: delivery type
		norm_delivery_type = user_input_from_limited_options("delivery", "Enter delivery type of order")
		bo.delivery_type = norm_delivery_type

		#Ask: name
		#bo.name = raw_input("Enter customer's name:")
		bo.name = user_input_with_validator("Enter customer's name for order (min 3 chars)", lambda s: len(s)>=3)
		#Ask: phone
		# s.isdigit()  looks better then use of all() in:  all(c.isdigit() for c in s)
		bo.phone_number = user_input_with_validator("Enter customer's phone number (digits only, min 5 chars)", \
		                                            lambda s: len(s)>=5 and s.isdigit() )

		#Ask: address
		# Choose right validator depending of delivery type:
		validator5c = lambda s: len(s)>=5
		validatorAny = lambda s: True
		validatorNow = validator5c if bo.delivery_type == "deliver" else validatorAny
		bo.address = user_input_with_validator("Enter customer's address (min 5 chars) (optional in case of collected order)",\
		                                       validatorNow)
		# end of Burger order entry! ==

		print "\nburger order: %d burger(s)" %len(bo.burgers_list)
		print "burger order total: $%.2f" %calc_burger_order_total(bo)
		database.append(bo)

	print "\n%d total order(s) in database at the moment.\n" %len(database)


def print_one_burger_details(b):
	print "Burger with:"
	print " bun:", b.bun_type
	s_addit_fillings= ", ".join(b.addit_fillings) if b.addit_fillings else "none"
	print " additional filling(s):", s_addit_fillings
	print

def print_order_details(bo):
	""" burger_order1 =
	   The details of each burger in the order,
     The total cost of the order, and
     The name, phone number and (if required) address of the person who made the order.
  """
	print "Order with %d burger(s):" %len(bo.burgers_list)
	for b in bo.burgers_list:
		print_one_burger_details(b)
	print "Customer's name:", bo.name
	print "Customer's phone number:", bo.phone_number
	print "Customer's address:", bo.address
	print " End of order"
	print

def print_database():
	print "All orders list: %d items" %len(database)
	for bo1 in database:
		print_order_details(bo1)
	print "End of orders database\n"

def main():
	"""
	Main operation cycle.
	ask operation:
  	enter a new order
	  show all orders
	  mark selected order as done (delivered or collected and money taken)
	"""
	while True:
		#print menu:
		menu = """Choose the operation, please:
 E(nter new order)
 S(how all orders)
 Q(uit)  /or empty/
 """

		# Read operation:
		cmd = raw_input(menu)
		if cmd=="":
			cmd="q"
		cmd = cmd[0].lower()

		if cmd=="q":
			break #and quit
		elif cmd=="e":
			enter_new_order()
		elif cmd=="s":
			print_database()
		else:
			print "\nWrong operation '%s' received, please try again." %(cmd)
	print "Done. Quitting.\n"
	return 0

# ===
def test():
	main()
	return True

if __name__=='__main__':
	""
	#test()
	main()
