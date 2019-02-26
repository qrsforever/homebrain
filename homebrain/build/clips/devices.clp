(defclass oic.d.bridge
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot connect_status (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot trigger_net (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.sosalarm
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot value (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.doorcontact
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot value (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.envsensor
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot humidity (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot illuminance (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot temperature (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.lightctrl
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot value1 (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot value2 (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot value3 (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.smartplug
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot value (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.scenectrl
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot scene (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.light
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot brightness (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot ct (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot value (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot hue (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot saturation (type NUMBER) (visibility public) (cardinality 2 2))
)

(defclass oic.d.letv
    (is-a DEVICE)
    (role concrete) (pattern-match reactive)
    (multislot power (type NUMBER) (visibility public) (cardinality 2 2))
    (multislot state (type NUMBER) (visibility public) (cardinality 2 2))
)

