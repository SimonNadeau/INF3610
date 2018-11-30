#include <hls_stream.h>
#include <ap_int.h>
#include <inttypes.h>

template<int D> struct myStream{
    ap_uint<D>       data;
    ap_uint<(D+7)/8> keep;
    ap_uint<(D+7)/8> strb;
    ap_uint<1>       user;
    ap_uint<1>       last;
};

typedef myStream<24> PIXEL_IN;
typedef myStream<16> PIXEL_OUT;
typedef hls::stream<PIXEL_IN> AXI_STREAM_IN;
typedef hls::stream<PIXEL_OUT> AXI_STREAM_OUT;

union PIX_OUT_DATA {
	struct {
		uint8_t first;
		uint8_t second;
	};
	uint16_t full;
};

void CrappyButFreeChromaResampler(AXI_STREAM_IN& video_in, AXI_STREAM_OUT& video_out)
{
#pragma HLS INTERFACE axis port=video_in
#pragma HLS INTERFACE axis port=video_out
#pragma HLS INTERFACE ap_ctrl_none port=return
	ap_uint<8> leftover = 0;
inf:while (true){
img:	for (int i = 0; i < 1920*1080; ++i) {
#pragma HLS pipeline
			bool sendChroma = (i % 2) == 0;
			auto pixIn = video_in.read();
			ap_uint<8> Y = pixIn.data & 0xFF;
			ap_uint<8> Cb = (pixIn.data >> 8) & 0xFF;
			ap_uint<8> Cr = (pixIn.data >> 16) & 0xFF;
			PIX_OUT_DATA pixOut;
			if (sendChroma) {
				pixOut.first = Y;
				pixOut.second = Cb;
				leftover = Cr;
			} else {
				pixOut.first = Y;
				pixOut.second = leftover;
			}

			PIXEL_OUT out;
			out.data = pixOut.full;
			out.keep = pixIn.keep;
			out.last = pixIn.last;
			out.strb = pixIn.strb;
			out.user = pixIn.user;

			video_out.write(out);
		}
	}
}
