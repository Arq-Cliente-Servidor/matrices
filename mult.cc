Matrix<float> MultBlock(const Matrix<float> &a, const Matrix<float> &b) {
  if (a.NumRows() == 2) {
    return a * b;
  } else {
    // Obtengo las mitades de cada matriz
    size_t a_nrows = a.NumRows() / 2;
    size_t a_ncols = a.NumCols() / 2;
    size_t b_nrows = b.NumRows() / 2;
    size_t b_ncols = b.NumCols() / 2;

    // Obtengo cada sub matriz de la matriz a
    Matrix<float> a00(a, 0, 0, a_nrows, a_ncols);
    Matrix<float> a10(a, a_nrows, 0, a_nrows, a_ncols);
    Matrix<float> a01(a, 0, a_ncols, a_nrows, a_ncols);
    Matrix<float> a11(a, a_nrows, a_ncols, a_nrows, a_ncols);

    // Obtengo cada sub matriz de la matriz b
    Matrix<float> b00(b, 0, 0, b_nrows, b_ncols);
    Matrix<float> b10(b, b_nrows, 0, b_nrows, b_ncols);
    Matrix<float> b01(b, 0, b_ncols, b_nrows, b_ncols);
    Matrix<float> b11(b, b_nrows, b_ncols, b_nrows, b_ncols);

    // Hago las multiplicaciones de cada cuadrante de la matriz
    Matrix<float> r00 = MultBlock(a00, b00) + MultBlock(a01, b10);
    Matrix<float> r10 = MultBlock(a10, b00) + MultBlock(a11, b10);
    Matrix<float> r01 = MultBlock(a00, b01) + MultBlock(a01, b11);
    Matrix<float> r11 = MultBlock(a10, b01) + MultBlock(a11, b11);

    // Creo la matriz resultante
    Matrix<float> result(a.NumRows(), b.NumCols());

    // Obtengo las referencias a cada cuadrante de la matriz
    Matrix<float *> r00r(result, 0, 0, a_nrows, b_ncols);
    Matrix<float *> r10r(result, a_nrows, 0, a_nrows, b_ncols);
    Matrix<float *> r01r(result, 0, b_ncols, a_nrows, b_ncols);
    Matrix<float *> r11r(result, a_nrows, b_ncols, r_nrows, b_ncols);

    // Guardo cada solucion de un cuadrante en la matriz resultante
    r00r = r00;
    r10r = r10;
    r01r = r01;
    r11r = r11;

    return result;
  }
};
