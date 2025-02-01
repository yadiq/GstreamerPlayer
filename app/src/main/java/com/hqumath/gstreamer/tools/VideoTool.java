package com.hqumath.gstreamer.tools;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;

public class VideoTool {
    public static final String TAG = "VideoTool";
    public static String softDecoder = "! decodebin3 ! videoconvert";//默认软解 高通软解H265存在问题，decodebin改decodebin3
    public static byte[] NALUStartCode = {0b00000000, 0b00000000, 0b00000000, 0b00000001};//H265 NALU单元开始标识
    public static int RtpSnLast;//RTP报文序列号

    /**
     * 手机解码器
     */
    public static String getDecoder(String endcoding) {
        String decoder;
        try {
            // 获取所有支持编解码器数量
            int numCodecs = MediaCodecList.getCodecCount();
            // 打印解码器
            /*for (int i = 0; i < numCodecs; i++) {
                MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
                if (!codecInfo.isEncoder())
                    LogUtil.d(TAG", "---" + codecInfo.getName());
            }*/
            for (int i = 0; i < numCodecs; i++) {
                // 编解码器相关性信息存储在MediaCodecInfo中
                MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
                // 判断是否为编码器
                if (!codecInfo.isEncoder()) {
                    if ("H265".equals(endcoding) || "h265".equals(endcoding)) {
                        //高通
                        if ("OMX.qcom.video.decoder.hevc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxqcomvideodecoderhevc";
                            return decoder;
                        }
                        //联发科
                        if ("OMX.MTK.VIDEO.DECODER.HEVC".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxmtkvideodecoderhevc";
                            return decoder;
                        }
                        //海思
                        if ("OMX.hisi.video.decoder.hevc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxhisivideodecoderhevc";
                            return decoder;
                        }
                        //全志, H40机顶盒
                        if ("OMX.allwinner.video.decoder.hevc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxallwinnervideodecoderhevc";
                            return decoder;
                        }
                        //瑞芯微, H50 T98机顶盒
                        if ("OMX.rk.video_decoder.hevc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxrkvideodecoderhevc";
                            return decoder;
                        }
                        //晶晨 小米box4机顶盒
                        if ("OMX.amlogic.hevc.decoder.awesome".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxamlogichevcdecoderawesome";
                            return decoder;
                        }
                        //海思上一代 荣耀6X麒麟655
                        if ("OMX.IMG.MSVDX.Decoder.HEVC".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omximgmsvdxdecoderhevc";
                            return decoder;
                        }
                        //猎户座Exynos VIVO X30,VIVO S6
                        if ("OMX.Exynos.HEVC.Decoder".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxexynoshevcdecoder";
                            return decoder;
                        }
                        //猎户座上一代 三星GALAXY S6 Edge+
                        if ("OMX.Exynos.hevc.dec".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxexynoshevcdec";
                            return decoder;
                        }
                    } else if ("H264".equals(endcoding) || "h264".equals(endcoding)) {
                        //高通
                        if ("OMX.qcom.video.decoder.avc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxqcomvideodecoderavc";
                            return decoder;
                        }
                        //联发科
                        if ("OMX.MTK.VIDEO.DECODER.AVC".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxmtkvideodecoderavc";
                            return decoder;
                        }
                        //海思
                        if ("OMX.hisi.video.decoder.avc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxhisivideodecoderavc";
                            return decoder;
                        }
                        //全志, H40机顶盒
                        if ("OMX.allwinner.video.decoder.avc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxallwinnervideodecoderavc";
                            return decoder;
                        }
                        //瑞芯微, H50 T98机顶盒
                        if ("OMX.rk.video_decoder.avc".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxrkvideodecoderavc";
                            return decoder;
                        }
                        //晶晨 小米box4机顶盒
                        if ("OMX.amlogic.avc.decoder.awesome".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxamlogicavcdecoderawesome";
                            return decoder;
                        }
                        //海思上一代 荣耀6X麒麟655 华为P9麒麟955
                        if ("OMX.IMG.MSVDX.Decoder.AVC".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omximgmsvdxdecoderavc";
                            return decoder;
                        }
                        //猎户座Exynos VIVO X30,VIVO S6
                        if ("OMX.Exynos.AVC.Decoder".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxexynosavcdecoder";
                            return decoder;
                        }
                        //猎户座上一代 三星GALAXY S6 Edge+
                        if ("OMX.Exynos.avc.dec".equals(codecInfo.getName())) {
                            decoder = "! parsebin ! amcviddec-omxexynosavcdec";
                            return decoder;
                        }
                    }
                }
            }
            return softDecoder;
        } catch (Exception e) {
            e.printStackTrace();
            return softDecoder;
        }
    }
}
