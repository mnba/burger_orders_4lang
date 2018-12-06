#| Unused:
;; This doesn't work
(defun list-append-elt (list1 elt)
  ;;(setf list1 (append list1 (list elt)))) ; this doesn't change list1
  (nconc list1 (list elt)) ; this either doesn't change list1 ;recycling op. 
  )
;; This works only for global (i.e. dinamic scope) vars;
;;   doesn't work with locally bound vars, since this globally defined function can't resolve
;;   list symbol to value.
;; list-append Usage: (list-append 'list-symb-here element)
(defun list-append(sym-list1 elt)
  (setf (symbol-value sym-list1) (append (symbol-value sym-list1) (list elt)) ))
|#
