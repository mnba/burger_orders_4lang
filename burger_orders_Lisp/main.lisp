
;;;
;;; Process burger orders, 4 languages. Common Lisp
;;; 

(defun create_strarray()
  '()
  ;;(make-array 6 :adjustable t :fill-pointer t :element-type 'string)
  )

;; either use defclass...
(defstruct OneBurger
		(bun_type "none")
		(addit_fillings ())
		(sum 0.))
;; create as:
(defun test_create_ob( &aux ob)
  ;;(defvar ob)
  (setf ob (make-oneburger :bun_type "standard" :addit_fillings '() :sum 2.)) 
  ;; getter:  
  (oneburger-addit_fillings ob) 
  ob)

(defstruct BurgerOrder
		(burgers_list '())
		(delivery_type "none")
		(name "")
		(phone_number "")
		(address "")
		(sum 0.))

(defun test_create_bo()
  (make-burgerorder :burgers_list '()
		    :delivery_type "collect"
		    :name "User1"
		    :phone_number "12345"
		    :address "str. aName"))

;; === Cost Calculation part ===

;; function by Rainer Joswig, stackoverflow site:

(defun create-hashtable (alist
                         &key (test 'equal) 
                         &aux (ht (make-hash-table :test test)))
  (loop for (key . value) in alist
        do (setf (gethash key ht) value))
  ht)

;;"Standard buns cost $2, multigrain buns cost $3 and brioche buns cost $4."
(defvar *bun_costs* ()) ;(make-hash-table :test #'equal))

(defun init_bun_costs()
  (setf *bun_costs*  
  	(create-hashtable 
  		'( ("standard" . 2.0) ("multigrain" . 3.0) ("brioche" . 4.0))
     )))

(init_bun_costs)

(defun get_bun_cost(bun_type)
	;;multivalue-bind...
	#|--(when (not bun_costs.has_key(bun_type)
		(raise Exception("Bun type unknown: "+bun_type))
		(return 0))-|#
	(gethash bun_type *bun_costs*)
)

(defun print-hash-entry (key value)
    (format t " ~S -> ~S~%" key value))
(defun print_dict(dict)
  (format t "Dict with ~a elements:~%" (hash-table-count dict))
  (maphash #'print-hash-entry dict) )

;global
;ok;(print_dict *bun_costs*)


(defvar *filling_costs* '())

(defun init_filling_costs()
  (setf *filling_costs*  
  	(create-hashtable 
  	  '(("beef pattie" . 1.0) ("bacon"  . 1.0)   ("lettuce" . 1.0)
       ("mushrooms" . 1.0)   ("pineapple" . 1.0)   ("jalapenos" . 1.0))
     )))

(init_filling_costs)

;g
;ok;(print_dict *filling_costs*)

(defun get_filling_cost(filling)
	;;multivalue-bind...
	#|if not filling_costs.has_key(filling):
		raise Exception("Filling unknown: "+filling)
		return 0|#
	(let ((cost (gethash filling *filling_costs*)))
	  (if (not cost)
	      (progn
	       ;;(error "Filling unknown: ~a" filling) ;
	       (format t "Filling unknown: ~a~%" filling)
	       0. ;ret 0
	       )
	      ;;else
	      cost)))
;test
(defun test_get_filling_cost()
  (format t "get filling cost1=~a~%" (get_filling_cost "mushrooms")) 
  (format t "get filling cost1=~a~%" (get_filling_cost "lettuce")) 
  (format t "get filling cost1=~a~%" (get_filling_cost "bacon")) 
  ;(format t "get filling cost1=~a~%" (get_filling_cost "")) 
  )

;ok
;(test_get_filling_cost)

;ok
(defun calc_one_burger_order(ob
			     &aux n sum)
  (setf n (length (oneburger-addit_fillings ob)))
  (if (> n 5)
    (error (format nil "Maximum 5 fillings are allowed, found ~a fillings" n)))
  
  (setf sum 0.)
  ;;All burgers come with a Beef pattie by default, at no extra cost.
  (incf sum (get_bun_cost (oneburger-bun_type ob))) ;+=
  ;;-for filling in (oneburger-addit_fillings ob):
  ;;-  sum+= get_filling_cost(filling)
  (loop for filling in (oneburger-addit_fillings ob) do
       (incf sum (get_filling_cost filling)))
  sum)

;; test it
(defun test_calc_one_burger_order()
  (let* 
      (#|(af-old (make-arrayuser_input_from_limited_options 3 :adjustable t :fill-pointer 0 
			    :initial-contents 
			    '("mushrooms" "lettuce" "bacon")
			    ))|#
       (af '("mushrooms" "lettuce" "bacon"))
       (ob (make-oneburger :bun_type "standard" :addit_fillings af :sum 3.15))
       price-ob1
       )
    (setf price-ob1 (calc_one_burger_order ob))
    (format t " price ob1= ~a~%" price-ob1)
  ))

;ok
;(test_calc_one_burger_order)


(defun calc_burger_order_total(bo
			       &aux sum)
  (setf sum 0.)
  ;for one_burger_order1 in bo.burgers_list:
  (loop for ob in (burgerorder-burgers_list bo) do 
	(incf sum (calc_one_burger_order ob)))
  ;; handle case of delivery type=deliver && order sum < $30.
  (if (and
       (equal (burgerorder-delivery_type bo) "deliver")
       (< sum 30.0))
    (incf sum 8.0)) ; add delivery fee
  (setf (burgerorder-sum bo) sum)
  sum)

;; === Data Entry and User input processing part part ===

;;Database Keeps all orders
(defparameter *database* '()) 
;; prev. impl.:
;;(make-array 5 :adjustable t :fill-pointer 0))
;; add things such way: (vector-push-extend 'bo *database*)

;; table for normalized names: maps type;option-selection-key string ->into-> normalized name
(defvar *norm_names_conslist* 
  '(
    ("bun;s" . "standard") ("bun;m" . "multigrain") ("bun;b" . "brioche")
    
    ("filling;b" . "beef pattie") ("filling;c" . "bacon") ("filling;l" . "lettuce") ("filling;m" . "mushrooms")
    ("filling;p" . "pineapple") ("filling;j" . "jalapenos")
    
    ("delivery;c" . "collect") ("delivery;d"."deliver")) )

(defvar *norm_names* 
  (create-hashtable *norm_names_conslist*))
;ok
(defun normalize_input(type inputstr )
  ;; convert input to normalized form
  ;; type is one of: bun type: "standard": 2.0, "multigrain": 3.0, "brioche": 4.0}
  (let* (
	 (type (string-downcase type))
	 (inputstrLow (string-downcase inputstr))
	 (key (concatenate 'string type ";" inputstrLow))
	 (sErr nil)
	 (norm_name "")
	 (val (gethash key *norm_names*))
	 )
    (if (not val)
	(progn 
	  (setf sErr (format nil "No such thing '~a' in ~a types~%" inputstr type))
	  (setf norm_name ""))
	(progn
	  (setf sErr nil)
	  (setf norm_name val)))
    (list sErr norm_name))) ;end of let, return 


(defun string-join(list1) ;delim param unused: hard-coded the ", " value
  (format nil "~{~A~^, ~}" list1))

;ok
(defun generate_menu(type
		     &aux 
		     (menu_list '())
		     (tslen (length type)) )
  ;;generate beautiful options from database names
  ;;"filling"
  ;;-(setf menu_list '())
  ;;-(setf tslen (length type)) ; type str len

  ;; iterating over hash:
  ;;for k,v in norm_names.items():
  (loop for k being the hash-keys of *norm_names*
     using (hash-value v)
     do 
       (when (and
	      (>= (length k) tslen)  ;k.length()>=tslen 
	      (equal (subseq k 0 tslen) type)
	      )
	 (let (
	       (menu_option (format nil "(~a)~a" (string-upcase (subseq v 0 1)) (subseq v 1)) )
	       )
	   ;;-(format t "  opt=~a," menu_option) 
	   ;;(setf menu_list (append menu_list (list menu_option))) ; or just use array; or use push in cycle+nreverse in the end
	   (push menu_option menu_list)
	   ) ))

  ;; works but changes right order, so commenting out
  ;;(setf menu_list (nreverse menu_list))
  
  ;;-(format t "~%menu_list= ~{~a~^, ~}|~%" menu_list)

  ;;create menu from menu_list
  (let ( (res (string-join menu_list))
	) 
    res) ;return
  )

;ok
(defun raw_input(menu)
  (format t menu)
  (read-line))

;ok
(defun user_input_from_limited_options(stype prompt_str
				       &optional (custom_menu nil) (custom_exit_key nil)
				       &aux sErr normalized_option menu key_for_option)
  (setf normalized_option "")
  ;;while True: 
  (do ()
      (nil)
    (if (not custom_menu)
      (setf menu (generate_menu stype))
      (setf menu custom_menu))

    (setf key_for_option (raw_input (concatenate 'string prompt_str ", " menu ": ")))
    (if custom_menu
      (if (equal (string-downcase key_for_option) custom_exit_key)
	  (return-from  user_input_from_limited_options custom_exit_key))) ;return!
    (let ((pair (normalize_input stype key_for_option))
	  )
      (setf sErr (first pair)) ;unbox
      (setf normalized_option (second pair)) ;unbox
      (when sErr
	(format t sErr)
	(go end) ;(continue)
	)
      );end of let
    ;; Entry was OK! exiting
    (return) ;break
    end ; for continue. do loop has its tagbody created automatically
    );end of do
  normalized_option) ;return

;ok
(defun user_input_with_validator(prompt_str validate_func
				 &aux user_input ok )
  (setf user_input "")
  ;;while True:
  (do ()
      (nil)
    (setf user_input (raw_input (concatenate 'string prompt_str ": ")))
    (setf ok (funcall validate_func user_input))
    (if ok
	;; Entry was OK! exiting
	;;i.e. break. no break in Lisps
	(return)
	;;else:
	(progn	;(when (not ok)
	  (format t " Wrong input, please try again~%")
	  ;;(go end) ;(continue) ; no continue in Lisps
	  )
      )
    );end of do
  user_input)
;;^ok


(defun string-isdigit (s)
  (every (lambda (c)(digit-char-p c)) s) )

;; main enter function 
(defun enter_new_order(
		       &aux bo
		       prompt_f menu_f_sExit_msg
		       norm_bun_type 
		       norm_filling 
		       norm_delivery_type
		       validator5c validatorAny validatorNow)
  (setf bo (make-burgerorder ))

  ;;while True:
  (do ((new_burger_msg "") ;decl vars
       (newQ "")
       (ob ) ; oneburger object
       )
      (nil) ;inf cycle

    (setf new_burger_msg "Enter N(ew) and <Enter> (or any other key) to add new burger into order, otherwise press just <Enter> when you done:~%")
    (setf newQ (raw_input new_burger_msg))
    (if (or 
	 (not newQ) 
	 (equal newQ ""))
      ;;finish entry of burgers
      (return)) ; break

    (setf ob (make-oneburger ))

    ;;Ask: bun type
    (setf norm_bun_type (user_input_from_limited_options "bun" "Enter bun type of burger"))
    (setf (oneburger-bun_type ob) norm_bun_type)

    ;;Ask: fillings
    ;i=0
    ;while i<5:
    (do ((i 0 (1+ i))
	 )
	((>= i 5) )
      ;;menu = generate_menu("filling")
      ;; Set here hand-coded value for menu, since generated doesn't do well in first letter collision case:
      (setf prompt_f "~%Enter additional filling for burger")
      (setf menu_f_sExit_msg "(B)eef pattie, ba(C)con, (M)ushrooms, (L)ettuce, (J)alapenos, (P)ineapple~% Or press 'e' for E(nough) of additional fillings")

      (setf norm_filling (user_input_from_limited_options "filling" prompt_f  menu_f_sExit_msg "e"))
      (if (equal norm_filling "e")
        (return)) ; break
      ;; Add filling to the burger
      ;;ob.addit_fillings.append(norm_filling)
      (push norm_filling (oneburger-addit_fillings ob)) 
      (format t "~a filling added~%" norm_filling)
      ;;-(incf i 1)
      ); end of do-5
    (format t "Finished with fillings: ~a additional fillings added." (length (oneburger-addit_fillings ob)))

    (push ob (burgerorder-burgers_list bo)) ;add ob to bo's list - prependin lisp
    (format t "~%Burger added to the order: price $~,2f~%~%" (calc_one_burger_order ob))
    );end of do-inf

  (when (> (length (burgerorder-burgers_list bo)) 0) ; when is 'multi-if'
    ;;Ask: delivery type
    (setf norm_delivery_type (user_input_from_limited_options "delivery" "Enter delivery type of order"))
    (setf (burgerorder-delivery_type bo) norm_delivery_type)

    ;;Ask: name
    ;;bo.name = raw_input("Enter customer's name:")
    (setf (burgerorder-name bo) 
	  (user_input_with_validator "Enter customer's name for order (min 3 chars)"
				    (lambda (s) (>= (length s) 3)))
	  )
    ;;Ask: phone
    ;; s.isdigit()  looks better then use of all() in:  all(c.isdigit() for c in s)
    (setf (burgerorder-phone_number bo) 
	  (user_input_with_validator "Enter customer's phone number (digits only, min 5 chars)"
				     (lambda (s) (and (>= (length s) 5)
						      (string-isdigit s))))
	  )
    
    ;;Ask: address
    ;; Choose right validator depending of delivery type:
    (setf validator5c (lambda (s) (>= (length s) 5)) )
    (setf validatorAny (lambda (s) t))

    (setf validatorNow (if (equal (burgerorder-delivery_type bo) "deliver")
			   validator5c 
			   validatorAny))
    (setf (burgerorder-address bo) 
	  (user_input_with_validator "Enter customer's address (min 5 chars) (optional in case of collected order)"  
				     validatorNow))
    ;; end of Burger order entry! ==

    (format t "~%burger order: ~a burger(s)~%" (length (burgerorder-burgers_list bo)))
    (format t "burger order total: $~,2f~%"  (calc_burger_order_total bo))
    (push bo *database*) ; append bo to database

    ) ; end of when

  (format t "~a total order(s) in database at the moment.~%" (length *database*))
  nil
    )
;; end of enter functions

;; printer functions:
(defun print_one_burger_details(b)
  (format t "Burger with:~% bun: ~a~%" (oneburger-bun_type b))
  (let ( (s_addit_fillings (if (oneburger-addit_fillings b)
			       (string-join (oneburger-addit_fillings b))
			       "none" ))
	)
    (format t " additional filling(s): ~a~%" s_addit_fillings))
  nil) ;return

(defun print_order_details(bo)
  " burger_order1 =
     The details of each burger in the order,
     The total cost of the order, and
     The name, phone number and (if required) address of the person who made the order.
  "
  (format t "Order with ~a burger(s):~%" (length (burgerorder-burgers_list bo)))
  (loop for b in (burgerorder-burgers_list bo) do 
     (print_one_burger_details  b))
  (format t "Customer's name: ~a~%" (burgerorder-name bo))
  (format t "Customer's phone number: ~a~%" (burgerorder-phone_number bo))
  (format t "Customer's address: ~a~%" (burgerorder-address bo))
  (format t " End of order~%~%")
)

(defun print_database()
  (format t "All orders list: ~a items~%" (length *database*))
  (loop for bo1 in *database* do 
       (print_order_details bo1))
  (format t "End of orders database~%"))

;;
(defun main_user_input()
  "
  Main operation cycle.
  ask operation:
    enter a new order
    show all orders
    mark selected order as done (delivered or collected and money taken)
  "
  ;;while True:
  (do ( (menu "")  ; declare vars
	(cmd "") )
      (nil)
    ;; Print menu:
    (setf menu "Choose the operation, please:
 E(nter new order)
 S(how all orders)
 Q(uit)  /or empty/
 ")
    ;; Read operation:
    (setf cmd (raw_input menu))
    (if (equal cmd "")
      (setf cmd "q"))
    (setf cmd (string-downcase (subseq cmd 0)))

    (cond 
      ((equal cmd "q")
        (return) ;break ;and quit
	)
      ((equal cmd "e")
       (enter_new_order)
       )
      ((equal cmd "s")
       (print_database)
       )
      (t ;else:
       (format t "~%Wrong operation '~a' received, please try again.~%" cmd)
       )
      )
    ); end of do
  (format t "Done. Quitting.~%")
  0)
;;

;; === test and main()
(defun test()
  (main_user_input)
  t)

(defun main()
  ;; init globals: moved to upper places, along with definition
  ;;(init_bun_costs)
  ;;(init_filling_costs)
  ;; end of globals init. 

  ;;(test)
  (main_user_input)
  (return-from main 0)

  ;; final version:
  (handler-case 
      (progn
	;;(test)
	(main_user_input))
    (error (se)
      (format t "Error ~a found. Exitting.~%" se)))
)

(main)
