(load "circuits/util.scm")

(define defpart-mux-n-circuits
  (lambda (n)
    (let ((in-wires (list-of-n-symbols 'in n)))
      `(defpart ,(string->symbol (string-append "mux-" (number->string n) "-circuits"))
         (,@(for in-wires (lambda (wire) `(in yellow ,wire)))
          (out yellow o)
          (in yellow control)
          (signal control-sig))

         ,@(append
             (for (zip in-wires (range n))
               (lambda (args) ((flip apply) args (lambda (in-wire i)
                 `(relay-equal ,in-wire o control control-sig ,i))))))))))
