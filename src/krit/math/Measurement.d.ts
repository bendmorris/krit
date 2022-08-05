declare enum MeasurementType {
    Absolute,
    Percent,
}

interface Measurement {
    type: MeasurementType;
    value: number;
    latest: number;
}

interface AnchoredMeasurement {
    value: Measurement;
    anchor: number;
    latest: number;
}
