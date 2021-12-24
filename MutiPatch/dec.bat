decoder_app.exe -i merge_bs.bin -o test_dec.yuv
FC /B test_rec.yuv test_dec.yuv >> cmp.txt
pause