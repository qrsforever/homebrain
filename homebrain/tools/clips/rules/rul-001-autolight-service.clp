(defrule rul-001-autolight-service
    (datetime ?clock $?ox)
    ?svr <- (service (name ?name &:(eq ?name  autolight))
        (skey ?key) (state ?st) (saves $?saves)
        (args ?envid ?devid ?target ?step $?args)
        (ntime ?tm &:(or (> ?clock ?tm) (neq ?st running)))
    )
    (object (is-a oic.d.envsensor)
        (ID ?id &:(eq ?id ?envid))
        (illuminance ?ill ?ill-old)
    )
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ?devid))
        (brightness ?brg ?brg-old)
        (value ?val ?val-old)
    )
  =>
    (bind ?c (create-rule-context (sym-cat rul-001-autolight-service : ?key)))
    (logi "service: " ?name ", skey: " ?key ", state: " ?st)
    (if (eq ?st stop)
     then
        (return)
    )
    (if (eq ?st start)
     then
        (bind ?val (send ?ins getData value))
        (if (= ?val 0)
         then
            (send ?c act-control ?did value 1)
        )
        (bind ?delta (* ?target 0.1))
        (bind ?low (- ?target ?delta))
        (bind ?high (+ ?target ?delta))
        (modify ?svr (state running) (saves ?high ?low))
        (return)
    )
    (modify ?svr (ntime (+ ?clock 200)))
    (logi "service: " ?name ", envsensor: " ?envid ", illuminance: " ?ill ", target: " ?target " brighness: " ?brg)
    (bind ?high (nth$ 1 ?saves))
    (bind ?low (nth$ 2 ?saves))
    (if (and (= ?val 1) (> ?ill ?high))
     then
        (bind ?nb (- ?brg ?step))
        (if (> ?nb 0)
         then
            (send ?c act-control ?did brightness ?nb)
         else
            (send ?c act-control ?did brightness 0)
            (send ?c act-control ?did value 0)
        )
        (return)
    )
    (if (< ?ill ?low)
     then
        (if (= ?val 0)
         then
            (send ?c act-control ?did value 1)
            (send ?c act-control ?did brightness ?step)
            (return)
        )
        (if (= ?brg 100)
         then
            (return)
        )
        (bind ?nb (+ ?brg ?step))
        (if (< ?nb 100)
         then
            (send ?c act-control ?did brightness ?nb)
         else
            (send ?c act-control ?did brightness 100)
        )
        (return)
    )
)