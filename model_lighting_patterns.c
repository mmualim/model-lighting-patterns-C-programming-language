#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LIGHTS 99 /* maximum number of lights that the park can have */
#define LIGHT_HEIGHT 8.5 /*standard height for all lights*/
#define GRID_SIZE 100 
#define BASE_RESULT_LENGTH 0/* The minimum accepted result from a read*/
#define MINIMUM_LX 1.0 /* the loweest acceptebale bound of illumination*/
#define PERCENTAGE 100
#define PI 3.14159265359
#define MAP_LENGTH 72
#define MAP_HEIGHT_WIDTH_RATIO 1.8

/*Represents a light in the */
struct Light{
    double x;
    double y;
    double lm;
};

/*Holds the dimensions of the park*/
struct Park{
    double max_x;
    double max_y;
};

/* function prototypes*/
void read_lights(struct Light lights[],int *num_of_lights,double *total_output,FILE *file);
void read_park_dimensions(struct Park *park_dimesions,FILE *file);
void read_line(FILE *file);
void print_section1(struct Park park_dimesions,int num_of_lights,double total_output);
void process_grid(struct Park park_dimesions,struct Light lights[],int num_of_lights);
double get_illumination(double center_x,double center_y,struct Light lights[],int num_of_lights);
void print_map(struct Park park_dimesions,struct Light lights[],int num_of_lights);
char get_character_mapping(double illumination);

int main(int argc, char *argv[]){
    
    /*check if the filename has been entered*/
    if(argc < 2){
        printf("Enter filename containing the park and light configuration\n");
        return -1;
    }

    /* open file*/
    FILE *file = fopen(argv[1],"r");
    if(file == NULL){
        printf("The file could not be opened !\n");
        return -1;
    }

    /*Create the storage varibales*/
    struct Light lights[MAX_LIGHTS];
    struct Park park_dimesions;
    int num_of_lights;
    num_of_lights  = 0;
    double total_output = 0;
    
    /*Read from file*/
    read_park_dimensions(&park_dimesions,file);
    read_lights(lights,&num_of_lights,&total_output,file);

    /* handles the three main sections of the program*/
    print_section1(park_dimesions,num_of_lights,total_output);
    process_grid(park_dimesions,lights,num_of_lights);    
    print_map(park_dimesions,lights,num_of_lights);   

    return 0;
}

/* Reads  a full line*/
void read_line(FILE *file){
    char line[30];
    fgets (line, 30, file);
}

/* Reads data of lights from file*/
void read_lights(struct Light lights[],int *num_of_lights,
    double *total_output,FILE *file){

    /*read the header*/    
    read_line(file);

    double x_value;
    double y_value;
    double lm_value;

    /*loop unitl end of file getting the three enries per line for light source*/
    while (fscanf(file,"%lf",&x_value) > BASE_RESULT_LENGTH ) {
        fscanf(file,"%lf",&y_value);
        fscanf(file,"%lf",&lm_value);

        /*Create a light and aupdate counter*/  
        lights[*num_of_lights].x = x_value;
        lights[*num_of_lights].y = y_value;
        lights[*num_of_lights].lm = lm_value; 
        *num_of_lights += 1;
        *total_output += lm_value; /* calculate total output of the lights*/
    }

    fclose(file);
}

/* Reads data of park dimensions from file*/
void read_park_dimensions(struct Park *park_dimesions,FILE *file){
    /*read the header*/
    read_line(file);
    
    /*read th maxX and maxY*/ 
    fscanf(file,"%lf",&park_dimesions->max_x);
    fscanf(file,"%lf",&park_dimesions->max_y);

    /*get the line break*/  
    read_line(file);
}

/* prints the first section */
void print_section1(struct Park park_dimesions,int num_of_lights,
    double total_output){
    
    printf("S1, north-east corner at x= %.1lfm, y=  %.1lfm\n",
                    park_dimesions.max_x,park_dimesions.max_y);
    printf("S1, number of lights =  %d \nS1, total light output =  %.1lflm\n",
                    num_of_lights,total_output);        

 
}

/* gets the areas of the park with light less that 1.0lx*/
void process_grid(struct Park park_dimesions,
    struct Light lights[],int num_of_lights){
    
    double illumination;
    double center_x;
    double center_y;
    double low_light_areas;
    low_light_areas = 0;

    /* get the center points of grid point (0,0)*/
    double cell_size_x = park_dimesions.max_x / GRID_SIZE; 
    double cell_size_y = park_dimesions.max_y / GRID_SIZE;
    double first_center_x = cell_size_x / 2;
    double first_center_y = cell_size_y / 2;

    /*nested loop to navigate through all the cells in the grid*/    
    for(int x_point = 0; x_point < GRID_SIZE; x_point++){
        for(int y_point = 0; y_point < GRID_SIZE; y_point++){
            /*get current center*/
            center_x = (cell_size_x * x_point) + first_center_x;
            center_y = (cell_size_y * y_point) + first_center_y;

            /* get illumination and check is aceptable*/
            illumination = get_illumination(center_x,center_y,lights,num_of_lights);
            if(illumination < MINIMUM_LX)  low_light_areas+=1;
        }
    }
    
    /*calculate the percantage of ow light areas*/
    double low_light_percentage = (low_light_areas/(GRID_SIZE * GRID_SIZE)) * PERCENTAGE;
  
    printf("\nS2, %.2lf x %.2lf grid, %dm x %dm cells\n",
        cell_size_x,cell_size_y,GRID_SIZE,GRID_SIZE);

    printf("S2, fraction of cell centroids below %.1lflx requirement = %.2lf%%\n",
        MINIMUM_LX,low_light_percentage);
    
}

/* calculates the total illumination of the give ncenter points*/
double get_illumination(double center_x,double center_y,
    struct Light lights[],int num_of_lights){

    double illumination = 0;
    /*for all lights calculate the ilummination*/    
    for(int i = 0; i < num_of_lights; i++){
        /* get the euclidean distance between the point and the light*/
        double dx = (center_x - lights[i].x);
        double dy = (center_y - lights[i].y);
        double r = sqrt((dx*dx) + (dy*dy));

        /* get illumniation by light lm/4 *Pi* ( r2 + h2 )*/
        double distance = (r * r) + (LIGHT_HEIGHT * LIGHT_HEIGHT);
        double current_illumination = lights[i].lm / (4 * PI * distance);
        illumination += current_illumination;
    }

    return illumination;
}

/* prints the illumination map in the screen*/
void print_map(struct Park park_dimesions,struct Light lights[],
    int num_of_lights){

    /*get the cell dimensions nad vertical height of the map*/    
    double cell_width = park_dimesions.max_x / MAP_LENGTH;
    double target_height = cell_width * MAP_HEIGHT_WIDTH_RATIO;
    int num_vertical_cells = round(park_dimesions.max_y / target_height);
    double cell_height = park_dimesions.max_y / num_vertical_cells;

    /** array to hold all the characters**/
    char map[MAP_LENGTH][num_vertical_cells];
    double illumination;

    /*get centers*/
    double center_x;
    double center_y;
    double first_center_x = cell_width / 2;
    double first_center_y = cell_height / 2;

     /*nested loop to navigate through all the cells in the grid*/   
    for(int x_point = 0; x_point < MAP_LENGTH; x_point++){
        for(int y_point = 0; y_point < num_vertical_cells; y_point++){
            /*get current center*/
            center_x = (cell_width * x_point) + first_center_x;
            center_y = (cell_height * y_point) + first_center_y;

            /*get illumination and the character mapping for it*/
            illumination = get_illumination(center_x,center_y,lights,num_of_lights);
            map[x_point][y_point] = get_character_mapping(illumination);
        }
    }

    printf("\nS3,  %d x  %d grid, %.2lfm x %.2lfm cells\n",MAP_LENGTH,
        num_vertical_cells,cell_width,cell_height);

    /*print the map , invert the counter to start at the hightest to print map in correct orientation*/ 
    for(int y_point = num_vertical_cells - 1; y_point >= 0; y_point--){
        printf("S3, "); 
        for(int x_point = 0 ; x_point < MAP_LENGTH ; x_point++){
            printf("%c",map[x_point][y_point]);
        }
        printf("\n");
    }
    
}

/* returns the correct character mapping for an illlumination*/
char get_character_mapping(double illumination){
    char result;

    if(illumination >= 10.0){
        result = '+';        
    }else if(illumination >= 9.0){
        result = ' ';
    }else if(illumination >= 8.0){
        result = '8';
    }else if(illumination >= 7.0){
        result = ' ';
    }else if(illumination >= 6.0){
        result = '6';
    }else if(illumination >= 5.0){
        result = ' ';
    }else if(illumination >= 4.0){
        result = '4';
    }else if(illumination >= 3.0){
        result = ' ';
    }else if(illumination >= 2){
        result = '2';
    }else if(illumination >= 1.0){
        result = ' ';
    }else{
        result = '-';
    }

    return result;
}