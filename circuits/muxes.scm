(load "circuits/util.scm")

(define defpart-mux-n-circuits
  (lambda (n)
    (let ((in-wires (list-of-n-symbols 'in n)))
      `(defpart ,(string->symbol (string-append "mux-" (number->string n) "-circuits"))
         (,@(for in-wires (lambda (wire) `(in yellow ,wire)))
          (out yellow o)
          (in yellow control)
          (signal control-sig))

         ,@(for (zip in-wires (range n))
             (lambda (args) ((flip apply) args (lambda (in-wire i)
               `(relay-equal ,in-wire o control control-sig ,i)))))))))

(define defpart-demux-n-circuits
  (lambda (n)
    (let ((out-wires (list-of-n-symbols 'out n)))
      `(defpart ,(string->symbol (string-append "demux-" (number->string n) "-circuits"))
         ((in yellow i)
          ,@(for out-wires (lambda (wire) `(out yellow ,wire)))
          (in yellow control)
          (signal control-sig))

         ,@(for (zip out-wires (range n))
             (lambda (args) ((flip apply) args (lambda (out-wire i)
               `(relay-equal i ,out-wire control control-sig ,i)))))))))
