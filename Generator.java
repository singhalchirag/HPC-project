

import java.io.*;
import java.util.*;

public class Generator {

  public static void main(String[] args) {
    // TODO Auto-generated method stub
    
    int graphSize = Integer.parseInt(args[0]);
    HashMap<Integer, List<Integer>> hash = new HashMap<Integer, List<Integer>>();
    Random rand = new Random();
    for(int i = 0; i < graphSize; i++){
      int  neighbours = rand.nextInt(5);
      
      List<Integer> list = hash.get(i);
      if(list == null){
        hash.put(i, new ArrayList<Integer>());
      }
      neighbours = neighbours - hash.get(i).size(); 
      while(neighbours > 0){
        
        int no = rand.nextInt(graphSize);
        
        //System.out.println("neighbours");
        //System.out.println(i);
        //System.out.println(neighbours);
        //System.out.println(no);
        if(checkPresent(hash.get(i), no) || no == i){
          continue;
        }
        hash.get(i).add(no);
        if(hash.get(no) == null){
          hash.put(no, new ArrayList<Integer>());
        }
        hash.get(no).add(i);
        neighbours--;
        
      }
    }
    try{
      PrintWriter writer = new PrintWriter("/home/constantine/Desktop/graph.txt", "UTF-8");
      for(int i: hash.keySet()){
        String toWrite = Integer.toString(i);
        List<Integer> list = hash.get(i);
        if(list.size() != 0){
          toWrite = toWrite + ",";
        }
        for(int j = 0; j < list.size(); j++){
          if(j == list.size() - 1){
            toWrite = toWrite + list.get(j);
          } else{
            toWrite = toWrite + list.get(j) + ",";
          }
           
        }
        writer.println(toWrite);
      }
      writer.close();
      
    } catch (IOException e) {
     // do something
    }
  }
  
  public static boolean checkPresent(List<Integer> list, int value){
    for(int i = 0; i < list.size(); i++){
      if(list.get(i) == value){
        return true;
      }
    }
    return false;
  }
}
