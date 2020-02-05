(load "circuits/util.scm")

(define defpart-mux-n-circuits
  (lambda (n)
    (let ((in-wires (list-of-n-symbols 'in n))
          (control-wires (list-of-n-symbols 'control n)))
      `(defpart ,(string->symbol (string-append "mux-" (number->string n) "-circuits"))
         (,@(for in-wires (lambda (wire) `(in yellow ,wire)))
          (out yellow o)
          (in yellow control)
          (signal control-sig))

         ,@(apply append
             (for (zip in-wires control-wires (range n))
               (lambda (args) ((flip apply) args (lambda (in-wire control-wire i)
                 (let ((in-buf (string->symbol (string-append (symbol->string in-wire) "-buf"))))
                   (list
                     `(red ,control-wire)
                     `(red ,in-buf)
                     `(decider control ,control-wire control-sig == ,i control-sig one)
                     `(buffer ,in-wire ,in-buf)
                     `(relay ,in-buf o ,control-wire control-sig))))))))))))
