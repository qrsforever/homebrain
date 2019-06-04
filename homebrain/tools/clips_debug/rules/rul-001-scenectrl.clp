(defrule rul-001-scenectrl
    ?ins <- (object (is-a oic.d.scenectrl)
        (ID ins-00124b00-1646-0823-d0d0)
        (scene ?m ?m-old)
    )
  =>
    (bind ?c (create-rule-context rul-001-scenectrl))
    (logi t "scenectrl mode: " ?m)
    (switch ?m
     (case 0 then (assert (switch-scene room1 none)))
     (case 1 then (assert (switch-scene room1 comehome)))
     (case 2 then (assert (switch-scene room1 getup)))
     (case 3 then (assert (switch-scene room1 sleep)))
    )
)
