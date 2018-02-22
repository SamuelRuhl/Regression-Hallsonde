#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NMAX 2000 

#define SHOW 1

#if(SHOW)
struct measure
{
	int countValues;
	//Temperature Digits
	float tempDigits[NMAX]; 
	//Offset value or Sensitivity value
	float value[NMAX];
	//Temperature Values in Celsius
	float tempCel[NMAX];
    //function values f(x) = mx + c
    float m; 
    float c;
    //funcion values f(x) = ax^2 + bx + c_2
    float a[2]; 
    float b[2]; 
    float c_2[2]; 
    //only for sensetiv function Si'(T)
    float sens_[NMAX];
    //Sigma
    float O ; 
};

//Prototyps
int countFileLines(char filename[]);
struct measure readData(char filename[]);
struct measure computeTempCel(struct measure data);
void printData(struct measure data);
struct measure linReg(struct measure data); 
float total(float *array, int size);
float expototal(float *array, int size, int exp);
float expomultipli(float *xi , float *yi , int size, int exp_x , int exp_y); 
float average(float total, int size);
struct measure quadReg(struct measure data,int index); 
struct measure deleteElement(struct measure data,int position);
struct measure plausibilitycheck(struct measure date);
struct measure runawayValue(struct measure data);
struct measure Sensetivity(struct measure data);
struct measure computSigma(struct measure data);
struct measure sigmaRunaway(struct measure data);

//Main function
int main(int argc, char const *argv[]){
    
    //read data in the struct variable measure
    struct measure sensitivData = readData("SensitivityRawData.txt");
    struct measure offsetData = readData("OffsetRawData.txt") ;
    //compute Temperature values out ot the tempdigits
    sensitivData = computeTempCel(sensitivData) ;
    offsetData = computeTempCel(offsetData) ;
    //Plausabilitycheck by Temperature values
    sensitivData = plausibilitycheck(sensitivData) ;
    offsetData = plausibilitycheck(offsetData) ; 
    
    printf("\n");
    //printData(sensitivData);
    //printData(offsetData);

    //calculate lineare regression for the offsetdata
    offsetData = linReg(offsetData) ; 

    //get and delete runaway values from temperature-related Offset behavior
    offsetData = runawayValue(offsetData) ;
    //calculate lineare regression for the sensitivedata
    sensitivData = linReg(sensitivData);
    //get and delet runaway values from the temperature-related sensitive behavior
    sensitivData = runawayValue(sensitivData) ;

    //do first square regression for offset data
    offsetData = quadReg(offsetData,0);
    //calculate sigma for offset data
    offsetData = computSigma(offsetData);
    //delete values which are more then two sigma away from the regression graph
    offsetData = sigmaRunaway(offsetData);
    //do secound square regression for offset data
    offsetData = quadReg(offsetData,1);
    

    
    //get sensetivity 
    sensitivData = Sensetivity(sensitivData) ;
    //do first square regression for sensitiv data
    sensitivData = quadReg(sensitivData,0) ; 
    //calculate sigma for sensitiv data
    sensitivData = computSigma(sensitivData) ;
    //delete these values which are more then two sigma away from the regression graph
    sensitivData = sigmaRunaway(sensitivData) ;
    //do secound square regression for sensitiv data
    sensitivData = quadReg(sensitivData,1) ;
    
    printf("\n   %f,%f,  ",offsetData.m,offsetData.c);
    printf("%f,%f,%f,  ",offsetData.a[0],offsetData.b[0],offsetData.c_2[0]);
    printf("%f,%f,%f,  ",offsetData.a[1],offsetData.b[1],offsetData.c_2[1]);
    
    printf("\n");

    printf("\n   %f,%f,  ",sensitivData.m,sensitivData.c);
    printf("%f,%f,%f,  ",sensitivData.a[0],sensitivData.b[0],sensitivData.c_2[0]);
    printf("%f,%f,%f,  ",sensitivData.a[1],sensitivData.b[1],sensitivData.c_2[1]);

    printf("\n");

	return 0;
}


//function to count lines in a file
int countFileLines(char filename[]){

    int numberOfLines = 0 ;
    char ch ;
	FILE *fb ;
    fb = fopen(filename,"r") ;
    
    if(fb != NULL){

    do{
		ch = fgetc(fb) ;
        
        if(ch == '\n') numberOfLines ++ ;

	}while(ch != EOF) ;
    }

	fclose(fb) ;

	return numberOfLines ;

}

//function to read Data in a measure struct variable  
struct measure readData(char filename[]){
	struct measure values ;
	values.countValues = countFileLines(filename) ;
	FILE *fb ;
    fb = fopen(filename,"r") ;
    
    if (fb != NULL)
    {
    	int i;
    	for (i = 0; i < countFileLines(filename); ++i)
    	{
    		fscanf(fb,"%f %f",&values.tempDigits[i],&values.value[i]);
    	}

    }else{
    	printf("File could not be opened\n");
    }
    fclose(fb) ;

    return values ;

}
//compute the Temprature Values 
struct measure computeTempCel(struct measure data){

	unsigned int i ; 

	for(i = 0; i < data.countValues ; i++){

     data.tempCel[i] = 0.3665 * data.tempDigits[i] - 280 ;

     }

     return data ;
 }

//print struct Values
 void printData(struct measure data){
 	for (int i = 0; i < data.countValues; ++i)
 	{
 	#define print1	printf("\n%d.\nTemperature %f Celsius\nTemperature ",i + 1,data.tempCel[i]);
    #define print2  printf("%f Digits\nOffset/Sensetiv %f\n",data.tempDigits[i],data.value[i]);
 	}
 }

//compute total of an array
float total(float *array, int size){
    float total = 0 ; 
    for(int i = 0 ; i < size; i++){
       total = total + array[i] ;
    }
    return total ;
}
//compute exponential  total of an array 
float expototal(float *array, int size,int exp){
    float total = 0 ; 
    for(int i = 0 ; i < size; i++){
       total = total + pow(array[i],exp) ;
    }
    return total ;
}
//multiply two float values with exponent arguments
float expomultipli(float *xi , float *yi , int size, int exp_x , int exp_y){
    float re = 0; 
    for(int i = 0; i < size ; i++){
       re = re + (pow(xi[i],exp_x) * pow(yi[i],exp_y)) ;
    }
    return re ; 
}
//calculate the average of an total value
float average(float total, int size){
    float average ; 
    average = total / size ; 
    return average ; 
}
//do a linear regression of an struct measure
struct measure linReg(struct measure data){
    float n = data.countValues ; 
    float w = total(&data.tempDigits[0],data.countValues) ; //w
    float v = total(&data.value[0],data.countValues) ;  //v
    float u = expototal(&data.tempDigits[0],data.countValues,2); //u
    float t = expomultipli(&data.tempDigits[0],&data.value[0],data.countValues,1,1);  //t
    data.m = (n * t - w * v ) / (n * u - pow(w,2)) ;
    data.c = -((w * t - u * v) / (n * u - pow(w,2))) ;

    return data ;

}
//do a square regression of an struct measure
struct measure quadReg(struct measure data, int index){
    int n = data.countValues ;
    float xi_average = average(total(&data.tempDigits[0],n),n);
    float xi2_average = average(expototal(&data.tempDigits[0],n,2),n) ;
    float xi3_average = average(expototal(&data.tempDigits[0],n,3),n) ;
    float xi4_average = average(expototal(&data.tempDigits[0],n,4),n) ;
    
    float yi_average = average(total(&data.value[0],n),n) ;
    float yi2_average = average(expototal(&data.value[0],n,2),n) ;

    float xi_yi_average = average(expomultipli(&data.tempDigits[0],&data.value[0],n,1,1),n);
    float xi2_yi_average = average(expomultipli(&data.tempDigits[0],&data.value[0],n,2,1),n) ; 

    data.a[index] = ((xi2_yi_average - yi_average * xi2_average) 
              * (xi2_average - pow(xi_average,2)) 
              - (xi_yi_average - xi_average * yi_average) 
              * (xi3_average - xi_average * xi2_average))
            /((xi4_average - pow(xi2_average,2)) 
                * (xi2_average - pow(xi_average,2))
                - pow(xi3_average - xi_average * xi2_average,2));

    data.b[index] = (xi_yi_average - yi_average * xi_average - data.a[index]
                 * (xi3_average - xi_average * xi2_average))
             / (xi2_average - pow(xi_average,2)) ; 

    data.c_2[index] = yi_average - data.a[index] * xi2_average - data.b[index] * xi_average ; 

    return data ; 
}
//delete elements of struct measure combination, and increment the arrays. 
struct measure deleteElement(struct measure data,int position){
    
    printf("\nno.%d\n",position);
    
    printf("Delete tempCel : %f \n",data.tempCel[position]);
    for(int i = position ; i < data.countValues; ++i){
        data.tempCel[i] = data.tempCel[i + 1] ; 
    }
    printf("Delete Value : %f \n",data.value[position]);
    for(int i = position ; i < data.countValues; ++i){
        data.value[i] = data.value[i + 1] ; 
    }
       
    printf("Delete tempDigits : %f \n",data.tempDigits[position]);
    for(int i = position ; i < data.countValues; ++i){
        data.tempDigits[i] = data.tempDigits[i + 1] ; 
    }
    printf("\n");
    --data.countValues; 
    

   
    return data ;
}
/*plausibility check of the temperature values from an struct measure
  and delete the impausible onces */ 
struct measure plausibilitycheck(struct measure data){ 
    for(int i = 0 ; i <= data.countValues; ++i){
        if((data.tempCel[i] < -40.0) || (data.tempCel[i] > 60.000)){
        data = deleteElement(data,i) ; 
        } 
    }
    return data ; 
}
/*delete 4 runaway values from a struct measure, with the biggest difference
  to the linear regression. Two values with the moste positive difference and 
  two with the most negative difference */
struct measure runawayValue(struct measure data){
    float tmpValue ;
    int index[4] ;
    float runawaypos[2] ; 
    float runawayneg[2] ; 
    float diff  = 0; 
    for (int i = 0; i < 4; ++i)
    {
        index[i] = 0 ; 
    }
    runawaypos[0] = 0 ;
    runawayneg[0] = 0 ; 

    for(int i = 0 ; i <= data.countValues ; i++){
        tmpValue = data.a[0] * pow(data.tempCel[i],2) + data.b[0] * data.tempCel[i] + data.c_2[0] ;
        diff = tmpValue - data.tempCel[i]; 
            if(diff > runawayneg[0]){
                runawayneg[0] = diff ; 
                index[0] = i ; 
            }
            if((diff < runawayneg[0]) && (diff > runawayneg[1])){
                runawayneg[1] = diff ; 
                index[1] = i ;
                } 
            diff  ; 
            if(diff < runawaypos[0]){
                runawaypos[0] = diff; 
                index[2] = i ; 
            }
            if((diff > runawaypos[0]) && (diff < runawaypos[1])){
                runawaypos[1] = diff ; 
                index[3] = i ; 
            }
        
        diff = 0 ; 
    }

    data = deleteElement(data,index[0]) ;
    data = deleteElement(data,index[1]) ;
    data = deleteElement(data,index[2]);
    data = deleteElement(data,index[3]) ;

    return data ; 
 
}
//do prepare sensetiv values for sensetivity function
struct measure Sensetivity(struct measure data){
    for (int i = 0; i < data.countValues ; ++i)
    {
        data.sens_[i] = data.value[i] - data.a[1] * pow(data.tempCel[i],2) - data.b[1] * data.tempCel[i] - data.c_2[1] ; 
    }
    return data ; 
}
//compute the varianz sigma
struct measure computSigma(struct measure data){
    data.O = 0 ; 
    float I_T ; 
    float mw = average(total(&data.value[0],data.countValues),data.countValues); 
        for (int i = 0; i < data.countValues; ++i)
        {
        I_T = mw - data.a[0] * data.tempDigits[i] * data.tempDigits[i] 
            - data.b[0] * data.tempDigits[i] - data.c_2[0] ;
        data.O = data.O + pow(I_T,2) ;
        }
        data.O = sqrt(data.O); 
    return data ; 
}
/*certain and delete all values which had a bigger diffrence then two sigma 
  from the square regression function*/
struct measure sigmaRunaway(struct measure data){
    float mw = average(total(&data.value[0],data.countValues),data.countValues); 
    float regValue ; 
        printf("\n%d\n",data.countValues) ; 
    for (int i = 0; i < data.countValues; ++i)
    {
        regValue = data.a[0] * pow(data.tempDigits[i],2) - data.b[0] * data.tempDigits[i] - data.c_2[0] ;
        if(((mw + 2 * data.O) < regValue) || ((mw - 2 * data.O) > regValue)){
           data = deleteElement(data,i) ;
           printf("\nSigma runaway\n");
        }
    }
  return data ; 
}

#endif
