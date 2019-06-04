(defrule rul-001-autolight-service
    ?svr <- (service 
        (name ?name &:(eq ?name autolight)) (state ?st) (args ?envid ?devid ?target $?args)
    )
    ; ins-00124b00-12d2-6263-d0d0
    (object (is-a oic.d.envsensor)
        (ID ?id &:(eq ?id ?envid))
        (illuminate ?ill ?ill-old)
    )
    ; ins-00a17a88-a01a-03a3-6a2b
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ?devid))
    )
  =>
    (bind ?c (create-rule-context rul-001-autolight-service))
    (if (eq ?st stop)
     then
        (retract ?svr)
        (return)
    )
    (bind ?delta (* ?target 0.1))
    (bind ?low (- ?target ?delta))
    (bind ?high (+ ?target ?delta))
    (bind ?bright (send ?ins getData brightness))
    (printout t "illuminate: " ?ill " target: " ?target " brightness: " ?bright crlf)
    (if (> ?ill ?high)
      then
            (if (> ?bright 0)
              then
                    (bind ?nv (integer (- ?bright (/ ?bright 2))))
                    (if (> ?nv 5)
                      then
                            (while (> ?bright ?nv)
                                  (bind ?bright (- ?bright 5))
                                  (send ?c act-control ?did brightness ?bright)
                            )
                      else
                            (send ?c act-control ?did brightness 0)
                    )
            )
            (return)
    )
    (if (< ?ill ?low)
      then
            (if (< ?bright 100)
              then 
                   (bind ?nv (integer (+ ?bright (/ (- 100 ?bright) 2))))
                   (if (< ?nv 95)
                     then
                            (while (< ?bright ?nv)
                                  (bind ?bright (+ ?bright 5))
                                  (send ?c act-control ?did brightness ?bright)
                            )
                     else
                           (send ?c act-control ?did brightness 100)
                  )
            )
            (return)
    )
)
