declare class NetRequest {
    constructor();

    url: string;
    message: string;

    setHeader(header: string): void;
    get(): Promise<NetResponse>;
    post(): Promise<NetResponse>;
}

interface NetResponse {
    responseCode: number;
    data: string;
}

declare class Net {
}
