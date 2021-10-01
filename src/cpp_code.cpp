#include<RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]
using namespace Rcpp;
using namespace arma;


//Extracts relevant window from matrix based on position of central pixel and window size
// [[Rcpp::export]]
NumericMatrix C_extract_window(NumericMatrix r, IntegerVector w, IntegerVector idx){
  int nr = w(0);
  int nc = w(1);
  
  int rast_row_center = idx(0);
  int rast_row_top = rast_row_center-(nr-1)/2;
  
  int rast_col_center= idx(1);
  int rast_col_left = rast_col_center-(nc-1)/2;
  
  IntegerMatrix r_idx(nr, nc);
  IntegerMatrix c_idx(nr, nc);
  for(int i=0; i < nr; ++i){
    for(int j=0; j < nc; ++j){
      r_idx(i,j) = i+rast_row_top;
      c_idx(i,j)=j+rast_col_left;
    }}
  NumericMatrix dat(nr, nc);
  for(int k=0; k < dat.size(); ++k){
    dat[k]= r(r_idx[k], c_idx[k]);
  }
  return dat; //extracted window as a matrix
}


//Subsets rows in a numeric mattrix according to a logical vector
// [[Rcpp::export]]
NumericMatrix subset_mat_rows(NumericMatrix x, LogicalVector idx) {
  NumericVector x2 = as<NumericVector>(x);
  LogicalVector idx2 = rep(idx, x.ncol());
  NumericVector out_vect = x2[idx2];
  NumericMatrix out_mat(sum(idx), x.ncol(), out_vect.begin());
  return out_mat;
}

//Checks to see if values of predictor are unique (excluding intercept col of 1's). 
//If any all have the same value, the matrix cannot be inverted to solve for OLS.
// [[Rcpp::export]]
bool C_Check_Xmat(NumericMatrix X){
  int nc = X.ncol();
  IntegerVector n_unique(nc-1);
  for(int i = 1; i < nc; ++i){
    NumericVector vals = X(_,i);
    NumericVector uni_vals = unique(vals);
    n_unique[i-1] = uni_vals.length();
  }
  int min_n = min(n_unique);
  if(min_n < 2){
    return FALSE;
  } else{
    return TRUE;
  }
  }

//Ordinary Least Squares (returns parameters and residuals)
// [[Rcpp::export]]
List C_OLS(arma::mat X, arma::mat Y){
  arma::mat Xt = trans(X); 
  arma::mat XtX_inv= inv(Xt * X);
  arma::mat H = X * XtX_inv * Xt;
  arma::mat Yhat = H * Y;
  NumericVector B= Rcpp::as<Rcpp::NumericVector>(wrap(XtX_inv * (Xt * Y)));
  NumericVector resid = Rcpp::as<Rcpp::NumericVector>(wrap(Yhat - Y));
  List output = List::create(_["B"]=B, _["resid"] = resid);
  return output;
}

//Ordinary Least Squares (only returns parameters)
// [[Rcpp::export]]
NumericVector C_OLS_params(arma::mat X, arma::mat Y){
  arma::mat Xt = trans(X); 
  arma::mat XtX_inv= inv(Xt * X);
  arma::mat H = X * XtX_inv * Xt;
  NumericVector B= Rcpp::as<Rcpp::NumericVector>(wrap(XtX_inv * (Xt * Y)));
  return B;
}

//Ordinary Least Squares (only returns residuals)
// [[Rcpp::export]]
NumericVector C_OLS_resid(arma::mat X, arma::mat Y){
  arma::mat Xt = trans(X); 
  arma::mat XtX_inv= inv(Xt * X);
  arma::mat H = X * XtX_inv * Xt;
  arma::mat Yhat = H * Y;
  NumericVector resid = Rcpp::as<Rcpp::NumericVector>(wrap(Yhat - Y));
  return resid;
}

//Multiscale metrics across matrix using sliding window
// [[Rcpp::export]]
List C_multiscale(NumericMatrix r, IntegerVector w, NumericMatrix X, int type, bool na_rm){
  int nr= r.nrow();
  int nc= r.ncol();
  int min_row = (w(0)-1)/2;
  int max_row = nr - ((w(0)-1)/2);
  int min_col = (w(1)-1)/2;
  int max_col = nc - ((w(1)-1)/2);
  //int center_idx = ((w[0]*w[1])-1)/2;
  
  //Z = aX2 + bY2 + cXY + dX + eY + f
  NumericMatrix a = NumericMatrix(nr,nc);
  a.fill(NA_REAL);
    
  NumericMatrix b = NumericMatrix(nr,nc);
  b.fill(NA_REAL);
  
  NumericMatrix c = NumericMatrix(nr,nc);
  c.fill(NA_REAL);
  
  NumericMatrix d = NumericMatrix(nr,nc);
  d.fill(NA_REAL);
  
  NumericMatrix e = NumericMatrix(nr,nc);
  e.fill(NA_REAL);
  
  NumericMatrix f = NumericMatrix(nr,nc);
  f.fill(NA_REAL);
  
  NumericMatrix SD_resid = NumericMatrix(nr,nc);
  SD_resid.fill(NA_REAL);
  
  //NEED AT LEAST 4/6 POINTS TO CALCULATE BECAUSE NEED AS MANY POINTS AS PARAMETERS
  int thresh = 6;
  if(type==1){
    thresh = thresh - 2;
  }
  for(int i = min_row; i< max_row; ++i) {
    for(int j = min_col; j < max_col; ++j){
      IntegerVector idx = IntegerVector(2);
      idx(0)= i;
      idx(1)=j;
      NumericMatrix curr_window = C_extract_window(r, w, idx);
      NumericVector Z = as<NumericVector>(curr_window);
      LogicalVector NA_idx = is_na(Z);
      int n_obs = sum(!NA_idx);

      if((is_true(any(NA_idx)) && (!na_rm)) || (n_obs < thresh)) {} else {
        NumericVector Z_trim_vect = Z[!NA_idx];
        NumericMatrix Z_trim(n_obs,1, Z_trim_vect.begin());
        NumericMatrix X_trim = subset_mat_rows(X, !NA_idx);
        
        bool can_be_inverted = C_Check_Xmat(X_trim);
        if(!can_be_inverted){} else{
          List OLS_fit = C_OLS(as<arma::mat>(X_trim), as<arma::mat>(Z_trim));
          NumericVector params = OLS_fit["B"];
          NumericVector resid =  OLS_fit["resid"];
          SD_resid(i,j) = sd(resid);
          if (type==2){
            a(i,j) = params[1];
            b(i,j) = params[2];
            c(i,j) = params[3];
            d(i,j) = params[4];
            e(i,j) = params[5];
            f(i,j) = params[0];
          } else {
            c(i,j) = params[1];
            d(i,j) = params[2];
            e(i,j) = params[3];
            f(i,j) = params[0];
          }}}}
        }
        

  List out= List::create(_["a"]=a, _["b"]=b, _["c"]=c, _["d"]=d, _["e"]=e, _["f"]=f, _["SD_resid"]=SD_resid);
  return(out);
}

//Multiscale metrics across matrix using sliding window (Quadratic Fit)
// [[Rcpp::export]]
NumericMatrix C_multiscale2(NumericMatrix r, IntegerVector w, NumericMatrix X, bool na_rm){
  int nr= r.nrow();
  int nc= r.ncol();
  int min_row = (w(0)-1)/2;
  int max_row = nr - ((w(0)-1)/2);
  int min_col = (w(1)-1)/2;
  int max_col = nc - ((w(1)-1)/2);
  int n_elem = nr * nc;
  //int center_idx = ((w[0]*w[1])-1)/2;
  
  //Z = aX2 + bY2 + cXY + dX + eY + f
  
  NumericMatrix out = NumericMatrix(n_elem, 7);
  colnames(out)= CharacterVector::create("a", "b", "c", "d", "e", "f", "mask");
  out.fill(NA_REAL);
  out(_,6)=rep(0,100); //initialize mask with 0's
  
  //NEED AT LEAST 6 POINTS TO CALCULATE BECAUSE NEED AS MANY POINTS AS PARAMETERS
  int thresh = 6;
  for(int i = min_row; i< max_row; ++i) {
    for(int j = min_col; j < max_col; ++j){
      IntegerVector idx = IntegerVector(2);
      idx(0)= i;
      idx(1)=j;
      NumericMatrix curr_window = C_extract_window(r, w, idx);
      NumericVector Z = as<NumericVector>(curr_window);
      LogicalVector NA_idx = is_na(Z);
      int n_obs = sum(!NA_idx);
      
      if((is_true(any(NA_idx)) && (!na_rm)) || (n_obs < thresh)) {} else {
        NumericVector Z_trim_vect = Z[!NA_idx];
        NumericMatrix Z_trim(n_obs,1, Z_trim_vect.begin());
        NumericMatrix X_trim = subset_mat_rows(X, !NA_idx);
        
        bool can_be_inverted = C_Check_Xmat(X_trim);
        if(!can_be_inverted){} else{
          int curr_elem_idx = i*nc + j; //rasters are indexed moving across rows
          NumericVector uni_Zvals = unique(Z_trim_vect);
          if(uni_Zvals.length() == 1){
            //If all Z values are the same, intercept should just be the value and all other parameters are 0. mask is 1 indicating all values are the same
            out(curr_elem_idx, 5) = uni_Zvals[0]; //f
            out(curr_elem_idx, 0) = 0; //a
            out(curr_elem_idx, 1) = 0; //b
            out(curr_elem_idx, 2) = 0; //c
            out(curr_elem_idx, 3) = 0; //d
            out(curr_elem_idx, 4) = 0; //e
            out(curr_elem_idx,6) = 1; //mask
            } else{
            NumericVector params = C_OLS_params(as<arma::mat>(X_trim), as<arma::mat>(Z_trim));
            out(curr_elem_idx, 5) =  params[0]; //f
            out(curr_elem_idx, 0) =  params[1]; //a
            out(curr_elem_idx, 1) =  params[2]; //b
            out(curr_elem_idx, 2) =  params[3]; //c
            out(curr_elem_idx, 3) =  params[4]; //d
            out(curr_elem_idx, 4) =  params[5]; //e
            }
          }}}
  }
  return(out);
}

//Multiscale metrics across matrix using sliding window (Planar Fit SD)
// [[Rcpp::export]]
NumericVector C_multiscale1(NumericMatrix r, IntegerVector w, NumericMatrix X, bool na_rm){
  int nr= r.nrow();
  int nc= r.ncol();
  int min_row = (w(0)-1)/2;
  int max_row = nr - ((w(0)-1)/2);
  int min_col = (w(1)-1)/2;
  int max_col = nc - ((w(1)-1)/2);
  int n_elem = nr * nc;
  //int center_idx = ((w[0]*w[1])-1)/2;
  
  //Z = cXY + dX + eY + f
  NumericVector out = NumericVector(n_elem); //SD Residuals
  out.fill(NA_REAL);
  
  //NEED AT LEAST 4 POINTS TO CALCULATE BECAUSE NEED AS MANY POINTS AS PARAMETERS
  int thresh = 4;
  for(int i = min_row; i< max_row; ++i) {
    for(int j = min_col; j < max_col; ++j){
      IntegerVector idx = IntegerVector(2);
      idx(0)= i;
      idx(1)=j;
      NumericMatrix curr_window = C_extract_window(r, w, idx);
      NumericVector Z = as<NumericVector>(curr_window);
      LogicalVector NA_idx = is_na(Z);
      int n_obs = sum(!NA_idx);
      
      if((is_true(any(NA_idx)) && (!na_rm)) || (n_obs < thresh)) {} else {
        NumericVector Z_trim_vect = Z[!NA_idx];
        NumericMatrix Z_trim(n_obs,1, Z_trim_vect.begin());
        NumericMatrix X_trim = subset_mat_rows(X, !NA_idx);
        
        bool can_be_inverted = C_Check_Xmat(X_trim);
        if(!can_be_inverted){} else{
          int curr_elem_idx = i*nc + j; //rasters are indexed moving across rows
          NumericVector uni_Zvals = unique(Z_trim_vect);
          if(uni_Zvals.length() == 1){
           out[curr_elem_idx] = 0; //SD resid
          } else{
            NumericVector resid = C_OLS_resid(as<arma::mat>(X_trim), as<arma::mat>(Z_trim));
            out[curr_elem_idx] =  sd(resid); //SD resid
          }
        }}}
  }
  return(out);
}
